(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (> (* (- 1) (+ (- 1) (* skoY skoX)) (+ 3 (* skoY skoY))) 0) (>  (+ (+ (+ (- 3465) (* skoX (* skoX (+ (- 4725) (* skoX (+ (- 4620) (* skoX (+ (- 1575) (* skoX (+ (- 3528) (* skoX (+ (- 75) (* skoX (- 300)))))))))))))) (* skoY (+ (* skoX (+ 3465 (* skoX (+ (- 13860) (* skoX (+ 4725 (* skoX (+ (- 14280) (* skoX (+ 1575 (* skoX (+ (- 2772) (* skoX 75))))))))))))) (* skoY (+ (+ (- 1155) (* skoX (+ (- 13860) (* skoX (+ (- 1575) (* skoX (+ (- 20440) (* skoX (+ (- 525) (* skoX (+ (- 7476) (* skoX (+ (- 25) (* skoX (- 400))))))))))))))) (* skoY (+ (- 4620) (* skoX (+ 1155 (* skoX (+ (- 10920) (* skoX (+ 1575 (* skoX (+ (- 6860) (* skoX (+ 525 (* skoX (+ (- 1024) (* skoX 25)))))))))))))))))))) (* skoZ (+ (+ (- 13860) (* skoX (* skoX (+ (- 18900) (* skoX (* skoX (+ (- 6300) (* skoX (* skoX (- 300)))))))))) (* skoY (+ (* skoX (+ 13860 (* skoX (* skoX (+ 18900 (* skoX (* skoX (+ 6300 (* skoX (* skoX 300)))))))))) (* skoY (+ (+ (- 4620) (* skoX (* skoX (+ (- 6300) (* skoX (* skoX (+ (- 2100) (* skoX (* skoX (- 100)))))))))) (* skoY (* skoX (+ 4620 (* skoX (* skoX (+ 6300 (* skoX (* skoX (+ 2100 (* skoX (* skoX 100)))))))))))))))))) 0) (<= (* (- 1) skoY) 0) (> (* (- 1) skoY) 0) (<= (* (- 1) (+ (+ (+ 1 (* skoX (- 1))) (* skoY (+ (- 1) (* skoX (- 1))))) (* skoZ (+ (- 1) (* skoY skoX))))) 0) (> (* (- 1) skoX) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (> (* (- 1) (+ (- 1) skoY)) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)