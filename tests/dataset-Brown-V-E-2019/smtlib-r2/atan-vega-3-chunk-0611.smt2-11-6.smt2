(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (=  skoX 0) (>  (+ (+ (* skoX (* skoX (* skoX (* skoX (* skoX (+ 252 (* skoX (* skoX 64)))))))) (* skoY (+ (* skoX (* skoX (* skoX (* skoX (+ 1260 (* skoX (* skoX 644))))))) (* skoY (+ (* skoX (* skoX (* skoX (+ 2520 (* skoX (* skoX (+ 2044 (* skoX (* skoX 192))))))))) (* skoY (+ 945 (* skoX (* skoX (+ 3885 (* skoX (* skoX (+ 3375 (* skoX (* skoX 675))))))))))))))) (* skoZ (+ (+ (+ (- 2835) (* skoX (* skoX (+ (- 1260) (* skoX (* skoX (+ 795 (* skoX (* skoX 128))))))))) (* skoY (+ (* skoX (+ 9450 (* skoX (* skoX (+ 7980 (* skoX (* skoX (+ 458 (* skoX (* skoX (- 128))))))))))) (* skoY (+ (+ 1890 (* skoX (* skoX (+ (- 4515) (* skoX (* skoX (+ (- 6270) (* skoX (* skoX (- 1253)))))))))) (* skoY (* skoX (+ (- 1890) (* skoX (* skoX (+ (- 2100) (* skoX (* skoX (- 450)))))))))))))) (* skoZ (+ (* skoX (+ 945 (* skoX (* skoX (+ 735 (* skoX (* skoX 64))))))) (* skoY (+ (+ 945 (* skoX (* skoX (+ (- 840) (* skoX (* skoX (+ (- 1245) (* skoX (* skoX (- 128)))))))))) (* skoY (+ (* skoX (+ (- 1890) (* skoX (* skoX (+ (- 1155) (* skoX (* skoX (+ 285 (* skoX (* skoX 64)))))))))) (* skoY (* skoX (* skoX (+ 945 (* skoX (* skoX (+ 1050 (* skoX (* skoX 225)))))))))))))))))) 0) (>  (+ (+ (+ 3 (* skoX skoX)) (* skoY (+ (* skoX (- 4)) (* skoY (+ 1 (* skoX (* skoX 3))))))) (* skoZ (+ (+ (* skoX 2) (* skoY (+ (+ 2 (* skoX (* skoX (- 2)))) (* skoY (* skoX (- 2)))))) (* skoZ (+ 1 (* skoY (+ (* skoX (- 2)) (* skoY (* skoX skoX))))))))) 0) (>  skoY 0) (=  (+ (+ (* skoX (* skoX (* skoX (- 1)))) (* skoY (+ (* skoX (* skoX (- 3))) (* skoY (* skoX (+ (- 3) (* skoX (* skoX (- 1))))))))) (* skoZ (+ (+ (- 3) (* skoX (* skoX (- 1)))) (* skoY (* skoX (+ 3 (* skoX skoX))))))) 0) (>  skoZ 0) (<  (+ (- 1) skoY) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)