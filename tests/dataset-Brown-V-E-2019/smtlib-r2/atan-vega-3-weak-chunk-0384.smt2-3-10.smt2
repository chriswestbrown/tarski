(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (>  skoX 0) (=  skoY 0) (=  (+ (+ (+ 399 (* skoX (+ 1200 (* skoX (+ 1330 (* skoX (+ 1200 (* skoX 399)))))))) (* skoY (+ (+ 1200 (* skoX (+ 1596 (* skoX (+ 1600 (* skoX 532)))))) (* skoY (+ (+ 1197 (* skoX (+ 1200 (* skoX (+ 798 (* skoX (+ 400 (* skoX 133)))))))) (* skoY (+ 900 (* skoX (* skoX (+ 600 (* skoX (* skoX 100)))))))))))) (* skoZ (+ (+ (+ 900 (* skoX (+ 2394 (* skoX (+ 2100 (* skoX 798)))))) (* skoY (+ (+ 2394 (* skoX (+ 1800 (* skoX (+ (- 1596) (* skoX (+ (- 1800) (* skoX (- 798))))))))) (* skoY (+ (+ 1800 (* skoX (+ (- 2394) (* skoX (+ (- 2100) (* skoX (+ (- 798) (* skoX (- 300))))))))) (* skoY (* skoX (+ (- 1800) (* skoX (* skoX (- 600))))))))))) (* skoZ (+ (+ 1197 (* skoX (+ 900 (* skoX 399)))) (* skoY (+ (+ 900 (* skoX (+ (- 2394) (* skoX (+ (- 1500) (* skoX (- 798))))))) (* skoY (+ (* skoX (+ (- 1800) (* skoX (+ 1197 (* skoX (+ 300 (* skoX 399))))))) (* skoY (* skoX (* skoX (+ 900 (* skoX (* skoX 300))))))))))))))) 0) (<  (+ (+ (* skoX (- 1)) (* skoY (- 1))) (* skoZ (+ (- 1) (* skoY skoX)))) 0) (>  skoZ 0) (<  (+ (- 1) skoY) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)
