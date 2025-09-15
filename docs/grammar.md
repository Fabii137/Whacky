```text
[Prog] -> [Stmt]*

[Stmt] -> { 
	bye([Expr])
	gimme ident = [Expr]
    ident = [Expr]
    maybe ([Expr])[Scope][MaybePred]
    [Scope]
}

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
    [Expr] / [Expr]     // prec = 3
    [Expr] * [Expr]     // prec = 3
    [Expr] - [Expr]     // prec = 2
    [Expr] + [Expr]     // prec = 2
    [Expr] < [Expr]     // prec = 1
    [Expr] <= [Expr]    // prec = 1
    [Expr] > [Expr]     // prec = 1
    [Expr] >= [Expr]    // prec = 1
    [Expr] == [Expr]    // prec = 1
    [Expr] != [Expr]    // prec = 1
    [Expr] and [Expr]   // prec = 0
    [Expr] or [Expr]    // prec = 0
}

[Term] -> {
    int_lit
    ident
    ([Expr])
}
```