import random #call the library of random
import string 

#first part is to get 8 random letters into a file

#first step it to assign 3 file names 

fileNames = ['green.txt', 'yellow.txt', 'red.txt']

# Second, we define a funtion to get random 10 letters from 26.
def randomLetters():
    return ''.join(random.choice(string.ascii_lowercase) for x in range(10))
# https://stackoverflow.com/questions/2823316/generate-a-random-letter-in-python is the source site for getting random letters


# next part is to open a file, write letters print and close it.
# We can make it as a loop of 3 to read all three files.

for f in fileNames:
    fPtr = open(f, 'w')      #open the files as while loop
    fPtr.write(randomLetters() + '\n') # write the files and add a space
    print( randomLetters()) #print the file
    fPtr.close()             #close the file.
    
#https://docs.python.org/2/tutorial/inputoutput.html is the source site 
#for the read, write funtion for python. 

      


#The second part of this assigment is to get 2 random numbers
#and the produtivity.It is not hard to do with.     
#https://docs.python.org/3/library/random.html is the source site.

#get random numbers from 1 to 42.(include 1 and 42)
x = random.randint(1, 42) 
y = random.randint(1, 42)

#get the product of x and y
z=x*y

#print out the vaule of x,y, and z
print(x)
print(y)
print(z)
