(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (= (* (+ 1000000000000000 (* skoX skoX)) (+ (- 3628800) (* skoY (* skoY (+ 1814400 (* skoY (* skoY (+ (- 151200) (* skoY (* skoY (+ 5040 (* skoY (* skoY (+ (- 90) (* skoY skoY)))))))))))))) skoX) 0) (>  (+ (+ 13933079999999970264000000000014364 (* skoX (* skoX (+ 51708676320000000000000000000 (* skoX (* skoX (- 1723665636))))))) (* skoY (* skoY (+ (* skoX (* skoX (+ 861840000000000000000000 (* skoX (* skoX 861840000))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (- 71820000000000000000000) (* skoX (* skoX (- 71820000)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ 2394000000000000000000 (* skoX (* skoX 2394000))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (- 42750000000000000000) (* skoX (* skoX (- 42750)))))) (* skoY (* skoY (* skoX (* skoX (+ 475000000000000000 (* skoX (* skoX 475)))))))))))))))))))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (* skoY (- 3)) pi) 0) (=  (+ (* skoY (- 4)) pi) 0) (=  (+ (- 120) skoX) 0) (=  (+ (- 100) skoX) 0)))
(check-sat)