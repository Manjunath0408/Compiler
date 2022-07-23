import os
import time

#   @echo "Running the First Code.."
# 	@echo "test1_IntAdd"
# 	@echo "----------------"
# 	./a.out < TEST/test1_IntAdd
# 	@echo "----------------"

inst=""
test_array={}
manual=[]
for test_file in os.listdir("TEST"):
    try:
        i=test_file.split("_")[0]
        i=i.replace("test","")
        i=int(i)
        test_array[i]=test_file
    except:
        manual.append(test_file)

if len(manual):
    print("Please check these programs as these are real world examples.....")
    print("\n")
    for i in manual:
        print(i)
    print("\n\n")
    print("********************************************************")
    time.sleep(3)




for test_file in sorted(test_array):
    os.system("clear")
    print(test_array[test_file])
    print("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^")
    os.system("cat TEST/"+test_array[test_file])
    print("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^")
    inst="./a.out < TEST/"+test_array[test_file]
    os.system(inst)
    inst='make runnasm'
    os.system(inst)
    time.sleep(3)


os.system("clear")
print("********************************************************")
print("Verified All the General Test Cases...")
print("********************************************************")