(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (=  (+ (+ (+ (- 28350) (* skoX (+ 51597 (* skoX (+ (- 37800) (* skoX 17199)))))) (* skoY (+ (+ 51597 (* skoX (+ (- 28350) (* skoX 17199)))) (* skoY (+ (+ (- 59850) (* skoX (+ 57330 (* skoX (+ (- 51450) (* skoX 19110)))))) (* skoY (+ (+ 57330 (* skoX (+ (- 22050) (* skoX (+ 19110 (* skoX 3150)))))) (* skoY (+ (+ (- 28800) (* skoX (+ 12285 (* skoX (+ (- 16350) (* skoX 4095)))))) (* skoY (+ (+ 12285 (* skoX (+ (- 1920) (* skoX (+ 4095 (* skoX 1610)))))) (* skoY (+ (- 1920) (* skoX (* skoX (- 640)))))))))))))))) (* skoZ (+ (+ 51597 (* skoX (+ (- 28350) (* skoX 17199)))) (* skoY (+ (+ (- 28350) (* skoX (+ (- 51597) (* skoX (+ 18900 (* skoX (- 17199))))))) (* skoY (+ (+ 57330 (* skoX (+ (- 3150) (* skoX (+ 19110 (* skoX 9450)))))) (* skoY (+ (+ (- 22050) (* skoX (+ (- 57330) (* skoX (+ 24150 (* skoX (- 19110))))))) (* skoY (+ (+ 12285 (* skoX (+ 15300 (* skoX (+ 4095 (* skoX 7350)))))) (* skoY (+ (+ (- 1920) (* skoX (+ (- 12285) (* skoX (+ 6110 (* skoX (- 4095))))))) (* skoY (* skoX (+ 1920 (* skoX (* skoX 640)))))))))))))))))) 0) (<  skoY 0) (<  skoX 0) (=  (+ (+ (+ 1 (* skoX (- 1))) (* skoY (+ (- 1) (* skoX (- 1))))) (* skoZ (+ (- 1) (* skoY skoX)))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)
