require 'eventmachine'
require 'pp'
require_relative './importRethinkDB.rb'

$port ||= (ARGV[0] || ENV['RDB_DRIVER_PORT'] || raise('driver port not supplied')).to_i
ARGV.clear
$c = r.connect(port: $port).repl

$gen = Random.new
puts "Random seed: #{$gen.seed}"

puts r.table_create('test').run rescue nil
$tbl = r.table('test')
$tbl.index_create('ts').run rescue nil
$tbl.index_wait.run

$ndocs = 20;
$ntries = 3;

$queries = [
  $tbl,
  # This makes the test get stuck. Investigate. (#5247)
  #$tbl.order_by(index: :id).limit(5),
  $tbl.order_by(index: :id),
  $tbl.map{|x| x.merge({'foo' => 'bar'})},
  $tbl.filter{|x| false},
  $tbl.filter{|x| r.branch(x[:id].type_of.eq("NUMBER"), x[:id].mod(2).eq(1), true)},
  $tbl.pluck(:id),
  $tbl.get_all(1),
  $tbl.get_all(1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
  $tbl.get_all(0, index: :ts),
  $tbl.between(5, 15),
  $tbl.between(1, 10, index: :ts),
  $tbl.between(0, 2, index: :ts)]

for nshards in [1, 2, 5]
  $tbl.reconfigure(replicas: 1, shards: nshards).run
  $tbl.wait.run
  for try in 1..$ntries
    $docs = (0...$ndocs).map{|i| {id: i, ts: 0}}
    puts $tbl.delete.run(durability: 'soft')
    puts $tbl.insert($docs).run(durability: 'soft')
    $tbl.rebalance.run
    $tbl.wait.run
    

    puts "\nRUNNING:"

    $num_running = $queries.size
    class Handler < RethinkDB::Handler
      def initialize(query)
        @query = query
        @log = []
        @state = {}
      end
      def query
        @query
      end
      def state
        @state
      end
      def log
        @log
      end
      def on_val(v)
        @log << v
        if v.key?('new_val') && v['new_val'] == nil && !v.key?('old_val')
          stop
          raise RuntimeError, "Bad val #{v}."
        elsif v.key?('old_val') && v['old_val'] == nil && !v.key?('new_val')
          stop
          raise RuntimeError, "Bad val #{v}."
        end
        @state.delete(v['old_val']['id']) if v['old_val']
        @state[v['new_val']['id']] = v['new_val'] if v['new_val']
      end
      def on_state(s)
        if s == 'ready'
          $num_running = $num_running - 1
        end
      end
    end

    $wlog = []
    EM.run {
      $opts = {include_initial: true, include_states: true, max_batch_rows: 1}
      $handlers = []
      for q in $queries
        $handlers.push(Handler.new(q))
        q.changes.em_run($handlers.last, $opts)
      end
      EM.defer {
        while $num_running != 0
          id = $gen.rand($ndocs)
          res = $tbl.get(id).delete.run
          $wlog << [id, res]
          res = $tbl.insert({ts: 1}).run
          $wlog << [id, res]

          id = $gen.rand($ndocs)
          res = $tbl.get(id).update({ts: 0.5}).run
          $wlog << [id, res]
          id = $gen.rand($ndocs)
          res = $tbl.get(id).update({ts: 3}).run
          $wlog << [id, res]
        end
        # Wait an extra 2 seconds to make sure that any pending changes have
        # been received.
        EM.add_timer(2) {
          for h in $handlers
            h.stop
          end
          EM.stop

          # Compare the state of each handler
          for h in $handlers
            actual = h.query.coerce_to(:array).run
            actual = actual.sort_by{|x| "#{x["id"]}"}
            state = h.state.values.sort_by{|x| "#{x["id"]}"}
            if "#{actual}" != "#{state}"
              print "Failed log:\n"
              PP.pp h.log
              raise RuntimeError, "State did not match query result.\n Query: #{h.query.pp}\n State: #{state}\n Actual: #{actual}"
            end
          end
        }
      }
    }
  end
end