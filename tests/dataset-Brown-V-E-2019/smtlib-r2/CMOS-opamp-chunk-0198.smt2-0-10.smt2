(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (>  (+ 1000000000000000 (* skoX skoX)) 0) (=  (+ 11975439168 (* skoY (* skoY (+ (- 5987520000) (* skoY (* skoY (+ 498960000 (* skoY (* skoY (+ (- 16632000) (* skoY (* skoY (+ 297000 (* skoY (* skoY (+ (- 3300) (* skoY (* skoY 25)))))))))))))))))) 0) (>  (+ (- 100) skoX) 0) (<  (+ (- 120) skoX) 0) (=  (+ (* skoY (- 4)) pi) 0) (=  (+ (* skoY (- 3)) pi) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0)))
(check-sat)
