(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoS3 () Real)
(declare-fun skoSX () Real)
(assert (and (<=  (+ (- 1) skoX) 0) (>  (+ (* skoS3 (+ (- 3840000) (* skoX (+ 21205665 (* skoX (+ (- 80100000) (* skoX (+ 98959770 (* skoX (+ (- 224700000) (* skoX (+ 89063793 (* skoX (- 151200000)))))))))))))) (* skoSX (+ (- 1280000) (* skoX (+ 7068555 (* skoX (+ (- 14700000) (* skoX (+ 32986590 (* skoX (+ (- 18900000) (* skoX 29687931)))))))))))) 0) (<=  (+ (* skoS3 (+ (- 300) (* skoX (+ 471 (* skoX (- 800)))))) (* skoSX (+ (- 100) (* skoX 157)))) 0) (=  (+ (+ (- 75) (* skoX (* skoX (- 80)))) (* skoSX skoSX)) 0) (=  (+ (- 3) (* skoS3 skoS3)) 0) (>  skoX 0) (>  skoSX 0) (>  skoS3 0)))
(check-sat)
