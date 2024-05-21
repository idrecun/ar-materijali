(set-logic QF_BV)
(declare-fun x () (_ BitVec 32))
(declare-fun y () (_ BitVec 32))
(declare-fun m () (_ BitVec 32))

(assert
 (and
	; Specifikacija fragmenta programa (P)
	(ite (bvsgt x y) (= m x) (= m y))

	; Negacija postuslova (~Q)
	(not (and
				(bvsge m x)
				(bvsge m y)
				(or (= m x) (= m y))
				))
 )
)

(check-sat)
(exit)
