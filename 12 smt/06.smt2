(set-logic QF_LIA)
(declare-fun x1 () Int)
(declare-fun x2 () Int)
(declare-fun x3 () Int)
(declare-fun x4 () Int)
(declare-fun x5 () Int)
(declare-fun x6 () Int)

(assert
 (and
	(< x1 x2 x3 x4 x5 x6)
	(distinct (- x2 x1) (- x3 x1) (- x4 x1) (- x5 x1) (- x6 x1)
	          (- x3 x2) (- x4 x2) (- x5 x2) (- x6 x2)
						(- x4 x3) (- x5 x3) (- x6 x3)
						(- x5 x4) (- x6 x4)
						(- x6 x5))
	(= (- x6 x1) 17)
 )
)

(check-sat)
(get-value (x1 x2 x3 x4 x5 x6))
(exit)
