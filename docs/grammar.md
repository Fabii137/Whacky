[Prog] -> [Stmt]*

[Stmt] -> { 
	exit([Expr])
	let ident = [Expr]
}

[Expr] -> {
    int_lit
    ident
    [BinExpr]
}

[BinExpr] -> {
    [Expr] * [Expr] // prec = 1
    [Expr] + [Expr] // prec = 0
}