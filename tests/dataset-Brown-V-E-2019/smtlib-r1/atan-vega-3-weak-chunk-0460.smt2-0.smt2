(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun skoZ () Real)
(assert (and (<= (* (- 1) skoY) 0) (> (* (- 1) skoY) 0) (<= (* (- 1) (+ 63 (* skoY (* skoY (+ 70 (* skoY (* skoY 15)))))) (+ (+ (* skoY (- 1)) (* skoX (- 1))) (* skoZ (+ (- 1) (* skoX skoY))))) 0) (<= (* (+ 63 (* skoY (* skoY (+ 70 (* skoY (* skoY 15)))))) (+ (+ (* skoY (- 1)) (* skoX (- 1))) (* skoZ (+ (- 1) (* skoX skoY))))) 0) (> (* (- 1) (+ (+ (* skoY (- 1)) (* skoX (- 1))) (* skoZ (+ (- 1) (* skoX skoY))))) 0) (> (* (- 1) (+ (+ (+ (- 9450) (* skoY (+ 17199 (* skoY (+ (- 19950) (* skoY (+ 19110 (* skoY (+ (- 9600) (* skoY (+ 4095 (* skoY (- 640))))))))))))) (* skoX (+ 17199 (* skoY (* skoY (+ 19110 (* skoY (+ 3150 (* skoY (+ 4095 (* skoY 1610))))))))))) (* skoZ (+ (+ 17199 (* skoY (+ (- 9450) (* skoY (+ 19110 (* skoY (+ (- 7350) (* skoY (+ 4095 (* skoY (- 640))))))))))) (* skoX (* skoY (+ (- 17199) (* skoY (+ 9450 (* skoY (+ (- 19110) (* skoY (+ 7350 (* skoY (+ (- 4095) (* skoY 640)))))))))))))))) 0) (<= (* (- 1) (+ (+ (+ 1 (* skoY (- 1))) (* skoX (+ (- 1) (* skoY (- 1))))) (* skoZ (+ (- 1) (* skoX skoY))))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (> (* (- 1) (+ (- 1) skoY)) 0) (> (* (- 1) (+ (* skoY (- 1)) skoX)) 0)))
(check-sat)
