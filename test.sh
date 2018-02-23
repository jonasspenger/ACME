echo "starting test";
# output from MDT -> remove lines starting with # -> compare with answer
build/ACME -i test/test_sequence.txt -f 2 -d 0 | grep -a -v '^#' | cmp test/test_sequence_f_2_d_0.answer;
build/ACME -i test/test_sequence.txt -f 2 -d 1 | grep -a -v '^#' | cmp test/test_sequence_f_2_d_1.answer;
build/ACME -i test/test_sequence.txt -f 3 -d 2 | grep -a -v '^#' | cmp test/test_sequence_f_3_d_2.answer;
build/ACME -i test/test_sequence.txt -f 5 -d 0 | grep -a -v '^#' | cmp test/test_sequence_f_5_d_0.answer;
echo "test completed";
