(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (>  skoX 0) (=  (+ (+ (* skoX (- 1)) (* skoY (- 1))) (* skoZ (+ (- 1) (* skoY skoX)))) 0) (=  (+ (+ (+ (- 9450) (* skoX (+ 17199 (* skoX (+ (- 19950) (* skoX (+ 19110 (* skoX (+ (- 9600) (* skoX (+ 4095 (* skoX (- 640))))))))))))) (* skoY (+ (+ 17199 (* skoX (+ (- 9450) (* skoX (+ 19110 (* skoX (+ (- 7350) (* skoX (+ 4095 (* skoX (- 640))))))))))) (* skoY (+ (- 9450) (* skoX (* skoX (+ (- 10500) (* skoX (* skoX (- 2250))))))))))) (* skoZ (+ (+ 17199 (* skoX (+ (- 9450) (* skoX (+ 19110 (* skoX (+ (- 7350) (* skoX (+ 4095 (* skoX (- 640))))))))))) (* skoY (+ (+ (- 9450) (* skoX (+ (- 17199) (* skoX (+ (- 1050) (* skoX (+ (- 19110) (* skoX (+ 5100 (* skoX (+ (- 4095) (* skoX 640)))))))))))) (* skoY (* skoX (+ 9450 (* skoX (* skoX (+ 10500 (* skoX (* skoX 2250))))))))))))) 0) (>  skoY 0) (>  skoZ 0) (<  (+ (- 1) skoY) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)