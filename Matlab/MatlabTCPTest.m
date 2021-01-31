if (exist('t') ~= 0)
    stop(t) %end previous Timers
end
IP = "192.168.137.14";

%these values control the timings of the measurement Process
measurementDuration_s = 2;
measurementInterval_s = 60;
counts = 10;
begin = now+minutes(0)+seconds(3);


assert(measurementInterval_s > measurementDuration_s*8, "it is not clear that there is enough time to send the data to Matlab");
assert(begin >= now+seconds(3), "begin is not far enouth in the Future");
sicherheitsfaktor_aufwachen = 5;
t = createTimer(begin,measurementInterval_s,counts,measurementDuration_s,sicherheitsfaktor_aufwachen, IP);
startat(t,begin)



function t = createTimer(begin,measurementInterval_s,counts,measurementDuration_s,sicherheitsfaktor_aufwachen, IP)
t = timer;
t.UserData = [];
t.StartFcn = {@initMeasurement,begin,measurementDuration_s};
t.TimerFcn = {@startMeasurement,begin, sicherheitsfaktor_aufwachen,measurementDuration_s, IP};
t.StopFcn = @endMeasurements;
t.Period = measurementInterval_s;
t.TasksToExecute = counts;
t.ExecutionMode = 'fixedRate';
end 

function initMeasurement(mTimer,~,begin,measurementDuration_s)
secondsPerMinute = 60;
secondsPerHour = 60*secondsPerMinute;
str1 = sprintf('Starting Measurement Timer at %s', datestr(begin,'yyyy.mm.dd HH:MM:SS'));
str2 = sprintf(' For the next %d:%d hours the Measurement will take', floor(mTimer.TasksToExecute*(mTimer.Period)/secondsPerHour),floor(mod(mTimer.TasksToExecute*(mTimer.Period),secondsPerHour)/60));
str3 = sprintf(' place every %d minutes.', mTimer.Period/secondsPerMinute);
str4 = sprintf(' for a duration of %d seconds.', measurementDuration_s);
disp([str1 str2 str3 str4])
end

function startMeasurement(mTimer,~,begin,sicherheitsfaktor_aufwachen,measurementDuration_s, IP)
    fprintf('starting µC connection at %s\n',datestr(now,'HH:MM:SS'));
    tcp = tcpclient(IP,23,"Timeout",mTimer.Period + 10);
    configureTerminator(tcp,13)

    while true
        a = convertCharsToStrings(char(readline(tcp)));
        if a == "start of transmission"
            break;
        end
    end
    fprintf('connection established. communication in progress\n')
    
    datesNotSend = 1;
    while datesNotSend
        datesNotSend = 0;
        dtnow = now;
        dtStart = begin + seconds(mTimer.Period*(mTimer.TasksExecuted-1)) + seconds(sicherheitsfaktor_aufwachen);
        assert(dtnow<dtStart,"Start liegt vor jetzt!!!!!")
        dtEnd = dtStart+minutes(0)+seconds(measurementDuration_s);
        dtNextMeeting = begin+seconds(mTimer.Period*mTimer.TasksExecuted) - seconds(sicherheitsfaktor_aufwachen);    

        
        writeline(tcp, "flush");
        flush(tcp);
        a = datestr(dtnow,'yyyymmddHHMMSS');
        writeline(tcp, a);
        b = erase(convertCharsToStrings(char(readline(tcp))),char(newline));
%         flush(tcp);
        if a ~= b
            datesNotSend = 1;
            fprintf("tried to send now: %s but got %s\n",a,b)
        end
        a = datestr(dtStart,'yyyymmddHHMMSS');
        writeline(tcp, a);
        b = erase(convertCharsToStrings(char(readline(tcp))),char(newline));
%         flush(tcp);
        if a ~= b
            datesNotSend = 1;
            fprintf("tried to send begin: %s but got %s\n",a,b)
        end
        a = datestr(dtEnd,'yyyymmddHHMMSS');
        writeline(tcp, a);
        b = erase(convertCharsToStrings(char(readline(tcp))),char(newline));
%         flush(tcp);
        if a ~= b
            datesNotSend = 1;
            fprintf("tried to send end: %s but got %s\n",a,b)
        end
        a = datestr(dtNextMeeting,'yyyymmddHHMMSS');
        writeline(tcp, a);
        b = erase(convertCharsToStrings(char(readline(tcp))),char(newline));
%         flush(tcp);
        if a ~= b
            datesNotSend = 1;
            fprintf("tried to send meeting: %s but got %s\n",a,b)
        end
    
    end
    fprintf('date sync sucessfull. waiting for data. Should start at around %s at the latest\n',datestr(dtStart + seconds(5*measurementDuration_s),'HH:MM:SS'))
    LinesReceived = 0;
    data = [];
    startet = 0;
    while 1
        if tcp.NumBytesAvailable > 0
            dataReceived = convertCharsToStrings(char(readline(tcp)));
            if startet == 0
                disp('receiving data....')
                startet = 1;
            end
            dataReceived = erase(dataReceived,char(newline));
            if dataReceived == "end of transmission"
                break;
            end
            data = [data dataReceived];
            LinesReceived = LinesReceived + 1;
        end 
    end
    clear tcp
    filename = ['Daten\' datestr(dtStart,'yyyymmdd_HH_MM_SS') '.csv'];
    fid = fopen(filename,'wt');
    fprintf(fid,'%s\n', data{:});
    fclose(fid);
    fprintf('%d lines recieved and Saved to %s\n', LinesReceived, filename);
    mTimer.UserData = [mTimer.UserData;LinesReceived];
    fprintf('%d lines in total total\n', sum(mTimer.UserData));
    if mTimer.TasksExecuted < mTimer.TasksToExecute
        fprintf('next meeting at %s\n',datestr(dtNextMeeting+seconds(sicherheitsfaktor_aufwachen),'HH:MM:SS'));
    end
    
    
    
end

function endMeasurements(mTimer,~)
    disp('al measurements taken')

    fprintf('%u LinesRecieved \n%u Lines Minimum\n%u Lines Maximum\n%u Lines Average', sum(mTimer.UserData), min(mTimer.UserData),max(mTimer.UserData),round(mean(mTimer.UserData)));
    
end
%     stop(t);