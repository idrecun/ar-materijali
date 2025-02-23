(set-logic QF_LRA)
(declare-fun x () Real)
(declare-fun y () Real)

(assert
 (and
	(= (+ (* 2 x) (* 3 y)) 7)
	(= (- (* 3 x) (* 6 y)) 7)
 )
)

(check-sat)
(get-value (x y))
(exit)
