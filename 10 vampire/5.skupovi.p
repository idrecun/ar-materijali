fof(jednakost, axiom,
    ![A, B]: (jednaki(A, B) <=> ![X]: (el(X, A) <=> el(X, B)))
   ).
fof(unija, axiom,
    ![A, B, U]: (unija(A, B, U) <=>
                 ![X]: (el(X, U) <=> el(X, A) | el(X, B))
                 )
    ).
fof(presek, axiom,
    ![A, B, P]: (presek(A, B, P) <=>
                 ![X]: (el(X, P) <=> el(X, A) & el(X, B)))
   ).
fof(komplement, axiom,
    ![A, B]: (komplement(A, B) <=>
              ![X]: (el(X, A) <=> ~el(X, B)))
   ).
fof(demorgan, conjecture,
    ![A, B, AC, BC, U, L, D]: (
      unija(A, B, U) &
      komplement(U, L) &
      komplement(A, AC) &
      komplement(B, BC) &
      presek(AC, BC, D) =>
      jednaki(L, D)
      )).
