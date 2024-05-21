% Asocijativnost i postojanje neutrala se mogu izostaviti
fof(asocijativnost, axiom,  ![X, Y, Z]: f(X, f(Y, Z)) = f(f(X, Y), Z) ).
fof(neutral, axiom, ![E]: (neutral(E) <=> ![X]: (f(X, E) = X & f(E, X) = X)) ).
fof(postoji_neutral, axiom, ?[E]: neutral(E) ).
fof(jedinstvenost, conjecture, ![E1, E2]: (
      neutral(E1) &
      neutral(E2) =>
      E1 = E2) ).
