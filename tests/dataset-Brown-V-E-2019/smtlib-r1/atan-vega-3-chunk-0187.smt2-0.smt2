(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (> (* (- 1) (+ (+ (+ 1 (* skoX (- 1))) (* skoY (+ (- 1) (* skoX (- 1))))) (* skoZ (+ (- 1) (* skoY skoX))))) 0) (>  (+ (+ (+ (- 300) (* skoX (+ 471 (* skoX (+ (- 400) (* skoX 157)))))) (* skoY (+ (+ 471 (* skoX (+ (- 300) (* skoX 157)))) (* skoY (+ (- 300) (* skoX (* skoX (- 100)))))))) (* skoZ (+ (+ 471 (* skoX (+ (- 300) (* skoX 157)))) (* skoY (+ (+ (- 300) (* skoX (+ (- 471) (* skoX (+ 200 (* skoX (- 157))))))) (* skoY (* skoX (+ 300 (* skoX (* skoX 100)))))))))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (> (* (- 1) (+ (- 1) skoY)) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)