import re
import os

src_dumpfile_name="./bin_dilithium2_clean/sign_dilithium2_riscv.elf.dump"
aim_function_list=['PQCLEAN_DILITHIUM2_CLEAN_ntt','PQCLEAN_DILITHIUM2_CLEAN_invntt_tomont']
output_path="./bin_dilithium2_clean/dump/"
inputfile_list=[]

if not os.path.exists(output_path):
    os.makedirs(output_path)

for aim_function in aim_function_list:
    src_dumpfile=open(src_dumpfile_name,"r",encoding='UTF-8')
    inputfile_name=output_path+"kem_kyber768_"+aim_function+".dump"
    inputfile=open(inputfile_name,"w",encoding='UTF-8')
    while True:
        line=src_dumpfile.readline()
        if not line:
            print("capture "+inputfile_name+" fail")
            break
        else:
            search=re.search('<'+aim_function+'>:',line)
            if not search:
                continue
            else:
                while line!='\n':
                    print(line,file=inputfile,end='')
                    line=src_dumpfile.readline()
                print("capture "+inputfile_name+" success")
                inputfile_list.append(inputfile_name)
                break
    src_dumpfile.close()
    inputfile.close()

exclusion_list=[] #不统计的“指令”列表，有些字符会被误认为是指令
for inputfile_name in inputfile_list:
    inputfile=open(inputfile_name,"r",encoding='UTF-8')
    outputfile_name=inputfile_name[:-5]+"_count.txt"
    outputfile=open(outputfile_name,"w",encoding='UTF-8')

    opcode_list={} #存储指令和它们调用次数的字典

    print(inputfile,file=outputfile)
    print(inputfile.readline(),file=outputfile) # 将第一行输出，分析指令时忽略第一行
    
    while True:
        #提取一行中“命令字符串”(可以包含字母、数字、下划线、点号，只能以字母、下划线开头)
        line=inputfile.readline()
        if not line:
            break
        elif len(line)<=29:
            continue
        else:
            command=line[29:] #命令从一行第32个字符开始到末尾
            opcode=""
            command_start_flag=0
            for char in command:
                if command_start_flag==0:
                    if char.isalpha() or char=="_":
                        opcode=opcode+char
                        command_start_flag=1
                    elif char=='.' or char.isalnum() or char=="#":
                        break
                elif command_start_flag==1:
                    if char.isalpha() or char.isalnum() or char=="." or char=="_":
                        opcode=opcode+char
                    else:
                        break
        
        # 将合法命令加入字典，如果命令已经在字典中，则更新其次数
        if opcode !="" and opcode not in exclusion_list:
            if opcode in opcode_list:
                opcode_list[opcode]=opcode_list[opcode]+1
            else:
                opcode_list[opcode]=1

    #字典按键排序
    opcode_list_sortbykey = dict(sorted(opcode_list.items(), key=lambda x: x[0]))

    #输出字典
    print("{0:<32s}{1:<s}".format("COMMAND","TIMES"),file=outputfile)
    for key in opcode_list_sortbykey:
        print("{0:<32s}{1:<,d}".format(key,opcode_list_sortbykey[key]),file=outputfile)

    #输出求和
    print('\n',end='',file=outputfile)
    print("{0:<32s}{1:<,d}".format("SUM",sum(opcode_list_sortbykey.values())),end='',file=outputfile)

    inputfile.close()
    outputfile.close()