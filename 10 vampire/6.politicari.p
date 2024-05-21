fof(politicari_lukavi, axiom, ![X]: (politicar(X) => lukav(X)) ).
fof(samo_pokvareni_su_politicari, axiom, ![X]: (~pokvaren(X) => ~politicar(X)) ).
fof(pokvaren_i_lukav, conjecture,
    ?[X]: politicar(X) => ?[Y]: (pokvaren(Y) & lukav(Y))
    ).
