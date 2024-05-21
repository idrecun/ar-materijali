(set-logic QF_LIA)
(declare-fun x () Int)
(declare-fun y () Int)
(declare-fun m () Int)

(assert
 (and
	; Specifikacija fragmenta programa (P)
	(=> (> x y) (= m x))
	(=> (not (> x y)) (= m y))

	; If-else moze i ovako
	; (ite (> x y) (= m x) (= m y))

	; Negacija postuslova (~Q)
	(not (and
				(>= m x)
				(>= m y)
				(or (= m x) (= m y))
				))
 )
)

(check-sat)
(exit)
