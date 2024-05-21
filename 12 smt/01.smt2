(set-logic QF_UF)
(declare-sort S 0)
(declare-fun f (S) S)
(declare-fun g (S) S)
(declare-fun a () S)
(declare-fun b () S)

(assert
 (and
	(= (f a) b)
	(= (g b) a)
	(distinct (g (f a)) a)
 )
)

(check-sat)
(exit)
