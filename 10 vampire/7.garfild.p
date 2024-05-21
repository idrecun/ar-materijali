fof(p1, axiom, ?[X]: (pas(X) & vlasnik(janko, X)) ).
fof(p2, axiom, ![X]: ((?[Y]: (pas(Y) & vlasnik(X, Y))) => voli_zivotinje(X)) ).
fof(p3, axiom, ![X]: (voli_zivotinje(X) =>
                      ~?[Y]: (zivotinja(Y) & udario(X, Y))) ).
fof(p4, axiom, (udario(janko, garfild) | udario(marko, garfild)) & macka(garfild) ).
fof(p5, axiom, ![X]: (macka(X) => zivotinja(X)) ).
fof(z, conjecture, udario(marko, garfild) ).
