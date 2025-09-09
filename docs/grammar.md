```text
[Prog] -> [Stmt]*

[Stmt] -> { 
	exit([Expr])
	let ident = [Expr]
}

[Expr] -> {
    [Term]
    [BinExpr]
}

[BinExpr] -> {
    [Expr] * [Expr] // prec = 1
    [Expr] + [Expr] // prec = 0
}

[Term] -> {
    int-lit
    ident
}
```