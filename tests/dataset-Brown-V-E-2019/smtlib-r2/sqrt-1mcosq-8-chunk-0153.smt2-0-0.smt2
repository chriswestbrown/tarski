(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(declare-fun skoX () Real)
(assert (and (=  (+ (- 2) (* skoY skoY)) 0) (=  (+ (- 174356582400) (* skoY (* skoY (+ 50854003200 (* skoY (* skoY (+ (- 3874590720) (* skoY (* skoY (+ 125405280 (* skoY (* skoY (+ (- 2210208) (* skoY (* skoY (+ 24388 (* skoY (* skoY (+ (- 184) (* skoY skoY)))))))))))))))))))) 0) (=  (+ (+ (- 2) (* skoY (- 10))) (* pi 5)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (=  (+ (- 1) (* skoX 10)) 0) (<  (+ (* skoY (- 1)) skoX) 0)))
(check-sat)