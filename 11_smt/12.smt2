; b = a[0]
; for(int i = 0; i < n - 1; i++) {
;   t = a[i];
;   a[i] = a[i + 1];
;   a[i + 1] = t;
; }
;
; Dokazati da je nakon izvrsavanja ovog fragmenta koda b == a[n-1]

(set-logic QF_ALIA)
(declare-fun a00 () (Array Int Int))
(declare-fun a01 () (Array Int Int))
(declare-fun a10 () (Array Int Int))
(declare-fun a11 () (Array Int Int))
(declare-fun a20 () (Array Int Int))
(declare-fun t1 () Int)
(declare-fun t2 () Int)
(declare-fun i0 () Int)
(declare-fun i1 () Int)
(declare-fun i2 () Int)
(declare-fun b () Int)

(assert
 (and
	; Program
	(= b (select a00 0))
	(= i0 0)

	(= i1 (+ i0 1))
	(= t1 (select a00 i0))
	(= a01 (store a00 i0 (select a00 (+ i0 1))))
	(= a10 (store a01 (+ i0 1) t1))

	(= i2 (+ i1 1))
	(= t2 (select a10 i1))
	(= a11 (store a10 i1 (select a10 (+ i1 1))))
	(= a20 (store a11 (+ i1 1) t2))

	; Negacija postuslova
	(not (= b (select a20 i2)))
 )
)

(check-sat)
(exit)
