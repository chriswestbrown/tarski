(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (> (* (- 1) (+ 63 (* skoX (* skoX (+ 70 (* skoX (* skoX 15))))))) 0) (> (* (- 1) (+ (+ (+ (- 2835) (* skoX (* skoX (+ (- 4095) (* skoX (* skoX (+ (- 1725) (* skoX (+ 1008 (* skoX (+ (- 225) (* skoX 256)))))))))))) (* skoY (+ (* skoX (+ 3780 (* skoX (* skoX (+ 4200 (* skoX (+ 5040 (* skoX (+ 900 (* skoX 2576)))))))))) (* skoY (+ (+ (- 945) (* skoX (* skoX (+ (- 3885) (* skoX (+ 10080 (* skoX (+ (- 3375) (* skoX (+ 8176 (* skoX (+ (- 675) (* skoX 768))))))))))))) (* skoY (+ 3780 (* skoX (* skoX (+ 15540 (* skoX (* skoX (+ 13500 (* skoX (* skoX 2700))))))))))))))) (* skoZ (+ (+ (+ (- 11340) (* skoX (+ (- 1890) (* skoX (+ (- 5040) (* skoX (+ (- 2100) (* skoX (+ 3180 (* skoX (+ (- 450) (* skoX 512)))))))))))) (* skoY (+ (+ (- 1890) (* skoX (+ 37800 (* skoX (+ (- 210) (* skoX (+ 31920 (* skoX (+ 1650 (* skoX (+ 1832 (* skoX (+ 450 (* skoX (- 512))))))))))))))) (* skoY (+ (+ 7560 (* skoX (+ 1890 (* skoX (+ (- 18060) (* skoX (+ 2100 (* skoX (+ (- 25080) (* skoX (+ 450 (* skoX (- 5012))))))))))))) (* skoY (* skoX (+ (- 7560) (* skoX (* skoX (+ (- 8400) (* skoX (* skoX (- 1800)))))))))))))) (* skoZ (+ (+ (- 945) (* skoX (+ 3780 (* skoX (+ (- 1050) (* skoX (+ 2940 (* skoX (+ (- 225) (* skoX 256)))))))))) (* skoY (+ (+ 3780 (* skoX (+ 1890 (* skoX (+ (- 3360) (* skoX (+ 2100 (* skoX (+ (- 4980) (* skoX (+ 450 (* skoX (- 512))))))))))))) (* skoY (+ (* skoX (+ (- 7560) (* skoX (+ (- 945) (* skoX (+ (- 4620) (* skoX (+ (- 1050) (* skoX (+ 1140 (* skoX (+ (- 225) (* skoX 256))))))))))))) (* skoY (* skoX (* skoX (+ 3780 (* skoX (* skoX (+ 4200 (* skoX (* skoX 900))))))))))))))))))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (> (* (- 1) (+ (- 1) skoY)) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)