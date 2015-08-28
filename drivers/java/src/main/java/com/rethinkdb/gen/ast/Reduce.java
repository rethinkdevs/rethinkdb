// Autogenerated by metajava.py.
// Do not edit this file directly.
// The template for this file is located at:
// ../../../../../../../../templates/AstSubclass.java

package com.rethinkdb.gen.ast;

import com.rethinkdb.gen.proto.TermType;
import com.rethinkdb.model.Arguments;
import com.rethinkdb.model.OptArgs;
import com.rethinkdb.ast.ReqlAst;



public class Reduce extends ReqlExpr {


    public Reduce(java.lang.Object arg) {
        this(new Arguments(arg), null);
    }
    public Reduce(Arguments args, OptArgs optargs) {
        this(null, args, optargs);
    }
    public Reduce(ReqlAst prev, Arguments args, OptArgs optargs) {
        this(prev, TermType.REDUCE, args, optargs);
    }
    protected Reduce(ReqlAst previous, TermType termType, Arguments args, OptArgs optargs){
        super(previous, termType, args, optargs);
    }


    /* Static factories */
    public static Reduce fromArgs(Object... args){
        return new Reduce(new Arguments(args), null);
    }


}