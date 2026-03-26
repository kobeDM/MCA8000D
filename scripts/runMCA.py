#!/usr/bin/python3
import os,subprocess
import datetime
import sys
from datetime import timezone, timedelta
JST = timezone(timedelta(hours=+9), 'JST')
import argparse
import threading
import influxdb
import time
import csv
import termios
#import mca8000d
#PATHs
HOME = os.environ["HOME"]+"/"
MCAdir=HOME+"MCA8000D/"
MCAbin=MCAdir+"bin/"
MCAConfigs=MCAdir+"MCAconfigfiles/"
MCAScripts=MCAdir+"scripts/"
MCARootMacros=MCAdir+"root_macros/"



readMCA="sudo "+MCAdir+"scripts/mca8000d.py"


# configs
CONFIG = "MCA_config.json"

stop_flag = False



print("**********************************************************")
print("   runMCA.py  ")
print("   MCA8000Ds DAQ for MIRACLUE-AIST2025  " )
print("   2025 Oct by K. Miuchi")
print("**********************************************************")

def make_new_period() -> str:
    p = 0
    while (os.path.isdir("per"+str(p))):
        p += 1        
    newper = "per" + str(p)
    cmd = "mkdir " + newper
    #print(cmd)
    subprocess.run(cmd, shell=True)
    return newper

def print_runsummary(file):
    id=0
    with open(file,'r') as f:
        reader=csv.reader(f,delimiter='\t')
        for data in reader:
            if(id==0):
                start=data[0]
            else:
                end=data[0]
            id=id+1
    print("from ",datetime.datetime.fromtimestamp(float(start)/1000.).astimezone(tz=JST).replace(tzinfo=JST))
    print("to ",datetime.datetime.fromtimestamp(float(end)/1000.).astimezone(tz=JST).replace(tzinfo=JST))
    print(str((float(end)-float(start))/1000.)," sec (real time).")
    
def post_to_influx(file,daemon):
    from influxdb import InfluxDBClient
    client = InfluxDBClient( host     = "10.37.0.214",port     = "8086",database= "miraclue" )
    #client = InfluxDBClient( host     = "10.37.0.170",port     = "8086",database= "miraclue" )
    if(not os.path.isfile(file)):
        cmd="touch "+file
        subprocess.run(cmd, shell=True)
    while(1):
        with open(file,'r') as f:
            reader=csv.reader(f,delimiter='\t')
            for data in reader:
                json_data = [
                    {
                        'measurement' : '3he_mca',
                        'fields' : {
                            'time_stamped'  : float(data[0]),
                            'event_rate_live'  : float(data[1]),
                            'rate_ROI1'  : float(data[2]),
                            'rate_ROI2'  : float(data[3]),
                            'rate_ROI3'  : float(data[4]),
                            'rate_ROI4'  : float(data[5])
                        },
                        'time': datetime.datetime.fromtimestamp(float(data[0])/1000.).astimezone(tz=JST).replace(tzinfo=JST).astimezone(tz=timezone.utc),
                        'tags' : {
                            'device' : '3he_mca'
                        }
                    }
                ]
                result = client.write_points(json_data)
        time.sleep(1)

def run_daq(args):
    global stop_flag
    config_filename = args.c
    presettime = args.presettime
    num_file_per_period = args.f
    ratefile="rate.txt"
            
    print("preset time for one file: "+str(presettime)+" sec.")
    print("number of files per period: "+str(num_file_per_period))
    
    # copy config file        
    if (not os.path.exists(config_filename)):
        print(config_filename+" does not exist.")
        cmd="cp "+MCAConfigs+CONFIG+" "+config_filename
        print(cmd)
        subprocess.run(cmd, shell=True)
    
    run=1
    #while(run):
    #while((stop_flag==False) and (quit_flag==False)):
    while(stop_flag==False):
        
        #if quit_flag :
        #    break
        #if stop_flag:
        #    print("s command was issued. stopping the DAQ after this file.")                #run=0
            #break

        new_per = make_new_period()        
        cmd="cp "+config_filename+" "+new_per
        #print(cmd)
        subprocess.run(cmd, shell=True)
        os.chdir(new_per)
        print("***********",new_per,"***********")
        #runend=0
        #thread1=threading.Thread(target=post_to_influx,args=(ratefile,"daemon"),daemon=True)
        #thread1.start()
        cmd=readMCA+" -c "+config_filename+" -p "+str(presettime)+" -f "+ str(num_file_per_period)
        print(cmd)
        cp=subprocess.run(cmd, shell=True)
        stop_flag=cp.returncode;
        #    print("\nrun end")
        #    print_runsummary(ratefile)
        #    run=0
        os.chdir("../")

        
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", help="config file name", default=CONFIG)
    parser.add_argument("-v","--verbose", help="verbose mode (control only)", action='store_true')
    parser.add_argument("-p","--presettime", help="preset time for one file", default=60)
    parser.add_argument("-f", help="num of files per period", default=60)
    args = parser.parse_args()

    running=1
    try:
        run_daq(args)
    except KeyboardInterrupt:
        print()
        print("===========================")
        print("aborted DAQ")
        print("===========================")

main()

