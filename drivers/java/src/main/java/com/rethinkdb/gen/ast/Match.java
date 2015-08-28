// Autogenerated by metajava.py.
// Do not edit this file directly.
// The template for this file is located at:
// ../../../../../../../../templates/AstSubclass.java

package com.rethinkdb.gen.ast;

import com.rethinkdb.gen.proto.TermType;
import com.rethinkdb.model.Arguments;
import com.rethinkdb.model.OptArgs;
import com.rethinkdb.ast.ReqlAst;



public class Match extends ReqlExpr {


    public Match(java.lang.Object arg) {
        this(new Arguments(arg), null);
    }
    public Match(Arguments args, OptArgs optargs) {
        this(null, args, optargs);
    }
    public Match(ReqlAst prev, Arguments args, OptArgs optargs) {
        this(prev, TermType.MATCH, args, optargs);
    }
    protected Match(ReqlAst previous, TermType termType, Arguments args, OptArgs optargs){
        super(previous, termType, args, optargs);
    }


    /* Static factories */
    public static Match fromArgs(Object... args){
        return new Match(new Arguments(args), null);
    }


}