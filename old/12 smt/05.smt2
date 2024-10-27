(set-logic QF_LIA)
(declare-fun x11 () Int)
(declare-fun x12 () Int)
(declare-fun x13 () Int)
(declare-fun x14 () Int)
(declare-fun x21 () Int)
(declare-fun x22 () Int)
(declare-fun x23 () Int)
(declare-fun x24 () Int)
(declare-fun x31 () Int)
(declare-fun x32 () Int)
(declare-fun x33 () Int)
(declare-fun x34 () Int)
(declare-fun x41 () Int)
(declare-fun x42 () Int)
(declare-fun x43 () Int)
(declare-fun x44 () Int)

; Sve vrednosti su razlicite
; Sve vrednosti su iz skupa {1, 2, ..., 16}
; Zbir u svakoj vrsti je 34
; Zbir u svakoj koloni je 34
; Zbirovi u dijagonalama su 34
; Neke vrednosti su vec popunjene

(assert
 (and

	(distinct x11 x12 x13 x14 x21 x22 x23 x24 x31 x32 x33 x34 x41 x42 x43 x44)

	(<= 1 x11 16)
	(<= 1 x12 16)
	(<= 1 x13 16)
	(<= 1 x14 16)
	(<= 1 x21 16)
	(<= 1 x22 16)
	(<= 1 x23 16)
	(<= 1 x24 16)
	(<= 1 x31 16)
	(<= 1 x32 16)
	(<= 1 x33 16)
	(<= 1 x34 16)
	(<= 1 x41 16)
	(<= 1 x42 16)
	(<= 1 x43 16)
	(<= 1 x44 16)

	(= (+ x11 x12 x13 x14) 34)
	(= (+ x21 x22 x23 x24) 34)
	(= (+ x31 x32 x33 x34) 34)
	(= (+ x41 x42 x43 x44) 34)

	(= (+ x11 x21 x31 x41) 34)
	(= (+ x12 x22 x32 x42) 34)
	(= (+ x13 x23 x33 x43) 34)
	(= (+ x14 x24 x34 x44) 34)

	(= (+ x11 x22 x33 x44) 34)
	(= (+ x14 x23 x32 x41) 34)

	(= x12 12)
	(= x22 8)
	(= x23 15)
	(= x31 7)
	(= x33 2)
	(= x41 4)
	(= x44 11)
 )
)

(check-sat)
(get-value (x11 x12 x13 x14))
(get-value (x21 x22 x23 x24))
(get-value (x31 x32 x33 x34))
(get-value (x41 x42 x43 x44))
(exit)

