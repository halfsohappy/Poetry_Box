import os

home_path = '/Users/bugsy/cyberannabel/Poetry_Box/public_poems/'
os.chdir(home_path)

naml = os.listdir();
naml.remove(".DS_Store")

for name in naml:
    sub = home_path+name+"/"
    print(sub)
    os.chdir(sub)
    for file_name in os.listdir(sub):
        os.rename(file_name, file_name + ".txt")