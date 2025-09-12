```text
[Prog] -> [Stmt]*

[Stmt] -> { 
	bye([Expr])
	let ident = [Expr]
    {[Stmt]*}
}

[Expr] -> {
    [Term]
    [BinExpr]
}

[BinExpr] -> {
    [Expr] / [Expr] // prec = 1
    [Expr] * [Expr] // prec = 1
    [Expr] - [Expr] // prec = 0
    [Expr] + [Expr] // prec = 0
}

[Term] -> {
    int_lit
    ident
    ([Expr])
}
```