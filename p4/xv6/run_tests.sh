python ~cs537-1/testing/p4/ThreadTest.py .
cat /dev/null > testsoutput.txt
echo "STARTING TESTS"
for ((a=1; a<= 10; a++))
  do
    echo "** run $a **"
    python ~cs537-1/testing/p4/ThreadTest.py . >> testsoutput.txt
  done
