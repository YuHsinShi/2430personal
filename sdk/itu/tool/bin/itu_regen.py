
# encoding: utf-8

import sys
from os.path import join, getsize
import os
import fnmatch
import errno
import time
import re
import shutil


def RemoveFile(filename):
    try:
        os.remove(filename)
    except OSError as e:
        if e.errno != errno.ENOENT:
            raise print("Remove file error.")
            sys.exit()


def getTotalFileSize(ts):
    size  = 0
    total = 0
    for f in range(len(ts)):
        size = getsize(ts[f])
        total += size

    return total


def partList(ts, gs, p, r):
    rIndex = gs * p
    lIndex = rIndex - gs
    print("P:      " + str(p))
    print("r:      " + str(r))
    print("lIndex: " + str(lIndex))
    print("rIndex: " + str(rIndex))

    size = 0
    index = 0
    result = []

    for index in range(len(ts)):
        if (size >= lIndex) and (size < rIndex):
            FileName, FileExtension = os.path.splitext(ts[index])
            f   = FileName + ".xml"
            S   = "project/"
            f   = f[(f.index(S) + len(S)):]

            #f = ts[index][0:-4] + ".xml"
            sz = "{0:,}".format(getsize(ts[index]))
            print('{0: >11}  {1: <}'.format(sz, f))  #ts[index][14:-4] + ".xml"
            result.append(FileName + ".xml")

        size += getsize(ts[index])
        if (size >= rIndex):
            break

    return result

def find(pattern, path):
    result = []
    for root, dirs, files in os.walk(path):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                result.append(os.path.join(root, name))
    return result

def searchXML(path, xmlList, ituList):
     if os.path.exists(path):
        for root, dirs, files in os.walk(path):
            for f in files:
                f = os.path.join(root, f)
                extname = f[-4:]
                if extname == ".itu":
                    if os.path.exists(f[:-4] + ".xml"):
                        xmlList.append(f[:-4] + ".xml")
                        ituList.append(f)


def calcGroup(p):
    p_re = re.compile(r"^(\d+)\/(\d+)$")
    match = p_re.search(p)
    print("Group: " + match.group())
    print("0: " + match.group(0))
    print("1: " + match.group(1))
    print("2: " + match.group(2))

    A = match.group(1)
    if A == 0:
        A = 1
    B = match.group(2)

    return [A, 1 / int(B)]

def writeIni(file):
    f = open("itu_regen.ini", "w+", encoding = 'UTF-8')
    f.write('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' + "\n")
    f.write('<ITURegenWork>' + "\n")
    f.write('    ' + '<JOB' + str(0) + ' XMLFILE="' +  file + "\" WORK=\"1\" JPEGQT=\"0\"/>\n")
    f.write('</ITURegenWork>' + "\n")
    f.close()

def copyItu(aFile, dest):
    f  = os.path.abspath(aFile)
    try:
        if os.path.exists(aFile):
            d = os.path.dirname(f)
            d_re = re.compile(r"project(\\|\/)(.+$)")
            match = d_re.search(d)

            if (dest[-1:] == "\\") or (dest[-1:] == "/"):
                dn = dest + match.group(2)
            else:
                dn = dest + "/" +  match.group(2)

            if not os.path.exists(dn):
                os.makedirs(dn, mode = 0o777)
                #print("Create dir: " + dn)

            #print("f:  " + f)
            #print("dn: " + dn)
            #print("d:  " + d)

            if (d != dn):
                shutil.copy(f, dn)
            #else:
                #print("The file can not be copied on itself.")

    except FileExistsError as e:
        print("FileExistsError: " + e.strerror)
    except FileNotFoundError as e:
        print("FileNotFoundError: " + e.strerror)
    except SameFileError as e:
        print("SameFileError: " + e.strerror)
    except OSError:
        print("OSError: " + e.strerror)


###############################################################################
#                                                                             #
#                                                                             #
###############################################################################


print("Current path: " + os.getcwd())
startTime  = time.time()
divConv    = -1
workPath   = os.path.dirname(sys.argv[0])
w          = workPath[0:-16]
outputPath = w + "regen_itu\\"
#print("w: " +  w)

if os.path.exists(outputPath) and os.path.isdir(outputPath):
    shutil.rmtree(outputPath)
    print("rmtree " + outputPath)

print("outputPath: " + outputPath)

ratio      = "1/1"

#print(workPath + "***")
print(outputPath)

#sys.exit()

if (os.path.exists(workPath)):
    os.chdir(workPath)
    if (os.path.exists("GUIDesigner.exe")):
        print("Find GUIDesigner")
    else:
        print("Not found GUIDesigner.")
        sys.exit()
else:
    print("Not fount the path, exit.")
    sys.exit()

for i in range(len(sys.argv)):
    s = sys.argv[i]
    text_re = re.compile(r"(\-\w)(.+)$")
    match = text_re.search(s)
    if(match):
        switch = match.group(1)
        value = match.group(2)
        print("switch: " + switch + ", value:  " + value)

        if (switch == "-p"):
            ratio = value
            print("ratio: " + ratio)

        if (switch == "-o"):
            outputPath = value
            print("output path: " + outputPath)
            if not (os.path.exists(outputPath)):
                try:
                    print("makedir " + outputPath)
                    os.makedirs(outputPath, mode=0o777)
                except:
                    print("make dir failed, exit")
                    sys.exit()

        #if (switch == "-h"):
        #    print("-p: run part/group mode")
        #    sys.exit()
    else:
        print("output: " + outputPath)

path = "../../../../project/"

print("find itu files in [%s]" % path)

ts     = []
ts_itu = []

RemoveFile("itu_regen.ini")

try:
    searchXML(path, ts, ts_itu)
    for i in range(len(ts)):
        FileName, FileExtension = os.path.splitext(ts[i])
        f   = FileName + FileExtension
        S   = "project/"
        f   = f[(f.index(S) + len(S)):]
        print('{0: <11}  {1: <}'.format("[ts]", f))

    for i in range(len(ts_itu)):
        FileName, FileExtension = os.path.splitext(ts[i])
        f   = FileName + ".itu"
        S   = "project/"
        f   = f[(f.index(S) + len(S)):]
        print('{0: <11}  {1: <}'.format("[ts_itu]", f))

    totalSize = int(getTotalFileSize(ts_itu))

    print("ratio: " + ratio)

    groupSize = int(totalSize * calcGroup(ratio)[1])
    part      = int(calcGroup(ratio)[0])

    print("part:       " + str(part))
    print('total size: {0:,}  {1:,}'.format(totalSize, groupSize))

    print("ts count    : " + str(len(ts)))
    print("ts_itu count: " + str(len(ts_itu)))

    ts = partList(ts_itu, groupSize, part, float(calcGroup(ratio)[1]))

    print("\n\nRun GUIDesigner to export itu.")
    c = str(len(ts))
    percent = 0
    for i in range(len(ts)):
        if os.path.exists(ts[i]):
            FileName, FileExtension = os.path.splitext(ts[i])
            f   = FileName + FileExtension
            S   = "project/"
            f   = f[(f.index(S) + len(S)):]
            source = FileName + ".itu"

            size = 0
            size = getsize(source) / groupSize
            percent = percent + size

            print("{0:>11,}/{1:<} [{2:>7,.2%}]: {3:<}".format((i + 1), c, percent, f))

            writeIni(ts[i])
            #print("ts[i]=" + ts[i])

            #if getsize(source) < (512 * 1024):
            os.system("GUIDesigner.exe")
            copyItu(source, outputPath)
            time.sleep(0.1)
        else:
            print("Not found the file: " + ts[i])
        #break

except OSError as e:
    print("Error: " + e.strerror)

endTime = time.time()
print("Time taken: {0:,.3f} seconds.".format(endTime - startTime))
os.system("PAUSE")
