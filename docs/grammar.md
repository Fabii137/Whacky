```text
[Prog] -> [Stmt]*

[Type] -> {
    number
    str
    bool
}

[Stmt] -> { 
    bye([Expr])
    gimme ident: [Type] = [Expr]
    ident = [Expr]
    maybe([Expr])[Scope][MaybePred]
    yell([Expr])
    loop(ident in [Expr]..[Expr])[Scope]
    thingy ident([ParamList]?): [Type] [Scope]
    gimmeback [Expr]
    why([Expr])[Scope]
    [Scope]
}

[ParamList] -> ident: [Type] (, ident: [Type])*

[Scope] -> {
   {[Stmt]*}
}

[MaybePred] -> {
    but([Expr])[Scope][MaybePred]
    nah[Scope]
    ε
}

[Expr] -> {
    [Term]
    [BinExpr]
}

[BinExpr] -> {
    [Expr] / [Expr]     // prec = 4
    [Expr] * [Expr]     // prec = 4
    [Expr] - [Expr]     // prec = 3
    [Expr] + [Expr]     // prec = 3
    [Expr] < [Expr]     // prec = 2
    [Expr] <= [Expr]    // prec = 2
    [Expr] > [Expr]     // prec = 2
    [Expr] >= [Expr]    // prec = 2
    [Expr] == [Expr]    // prec = 2
    [Expr] != [Expr]    // prec = 2
    [Expr] xor [Expr]   // prec = 1
    [Expr] band [Expr]  // prec = 1
    [Expr] bor [Expr]   // prec = 1
    [Expr] and [Expr]   // prec = 0
    [Expr] or [Expr]    // prec = 0
}

[Term] -> {
    int_lit
    bool                // yep / nope
    string
    ident
    ([Expr])
    ident([ArgList]?)
}

[ArgList] -> [Expr] (, [Expr])*

```