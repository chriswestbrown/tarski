(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (=  (+ 43589145600 (* skoX (* skoX (+ (- 3632428800) (* skoX (* skoX (+ 121080960 (* skoX (* skoX (+ (- 2162160) (* skoX (* skoX (+ 24024 (* skoX (* skoX (+ (- 182) (* skoX skoX))))))))))))))))) 0) (<  (+ (- 87178291200) (* skoX (* skoX (+ 43589145600 (* skoX (* skoX (+ (- 3632428800) (* skoX (* skoX (+ 121080960 (* skoX (* skoX (+ (- 2162160) (* skoX (* skoX (+ 24024 (* skoX (* skoX (+ (- 182) (* skoX skoX)))))))))))))))))))) 0) (>  (+ (* skoX (- 1)) skoY) 0) (=  (+ (- 1) (* skoX 10)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (=  (+ (+ (- 2) (* skoY (- 10))) (* pi 5)) 0)))
(check-sat)
