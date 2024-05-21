(set-logic QF_LIA)
(declare-fun x () Int)
(declare-fun y () Int)

(assert
 (and
	(> (+ (* 2 x) (* 3 y)) 1)
	(< (- (* 3 x) (* 2 y)) 0)
 )
)

(check-sat)
(get-value (x y))
(exit)
