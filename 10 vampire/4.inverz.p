fof(asocijativnost, axiom, ![X, Y, Z]: f(X, f(Y, Z)) = f(f(X, Y), Z) ).
fof(neutral, axiom, ![X]: (f(e, X) = X & f(X, e) = X) ).
fof(inverz, axiom, ![X, X]: (inverz(X, Y) <=> f(X, Y) = e & f(Y, X) = e) ).
fof(jedinstvenost, conjecture,
    ![X, Y1, Y2]: (inverz(X, Y1) & inverz(X, Y2) => Y1 = Y2)
    ).
