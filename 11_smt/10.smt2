; if(x < y && !(x + z < y + z))
;   ...
; Da li je moguce uci u ovu granu if-a

(set-logic QF_BV)
(declare-fun x () (_ BitVec 8))
(declare-fun y () (_ BitVec 8))
(declare-fun z () (_ BitVec 8))

(assert
 (and
	(bvslt x y)
	(not (bvslt (bvadd x z) (bvadd y z)))
 )
)

(check-sat)
(get-value (x y z))
(exit)
