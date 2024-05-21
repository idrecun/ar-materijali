(set-logic QF_LIA)
(declare-fun x1 () Int)
(declare-fun x2 () Int)
(declare-fun x3 () Int)
(declare-fun x4 () Int)
(declare-fun x5 () Int)
(declare-fun y1 () Int)
(declare-fun y2 () Int)
(declare-fun y3 () Int)
(declare-fun y4 () Int)
(declare-fun y5 () Int)

; Ne postoje dve kraljice u istoj vrsti
;  svi x-evi su razliciti

; Ne postoje dve kraljice u istoj dijagonali
;  zbir koordinata je razlicit

(assert
 (and
	(<= 1 x1 5)
	(<= 1 x2 5)
	(<= 1 x3 5)
	(<= 1 x4 5)
	(<= 1 x5 5)
	(<= 1 y1 5)
	(<= 1 y2 5)
	(<= 1 y3 5)
	(<= 1 y4 5)
	(<= 1 y5 5)

	(distinct x1 x2 x3 x4 x5)
	(distinct y1 y2 y3 y4 y5)
	(distinct (+ x1 y1) (+ x2 y2) (+ x3 y3) (+ x4 y4) (+ x5 y5))
	(distinct (- x1 y1) (- x2 y2) (- x3 y3) (- x4 y4) (- x5 y5))
 )
)

(check-sat)
(get-value (x1 y1))
(get-value (x2 y2))
(get-value (x3 y3))
(get-value (x4 y4))
(get-value (x5 y5))
(exit)

