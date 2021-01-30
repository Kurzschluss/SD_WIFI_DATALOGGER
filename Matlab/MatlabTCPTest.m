
begin = now+minutes(0)+seconds(3);
measurementDuration_s = 2;
measurementInterval_s = 60;
assert(measurementInterval_s > measurementDuration_s*8);
counts = 3;
sicherheitsfaktor_aufwachen = 5;


t = createTimer(begin,measurementInterval_s,counts,measurementDuration_s,sicherheitsfaktor_aufwachen);
startat(t,begin)



function t = createTimer(begin,measurementInterval_s,counts,measurementDuration_s,sicherheitsfaktor_aufwachen)
t = timer;
t.UserData = [];
t.StartFcn = {@initMeasurement,begin,measurementDuration_s};
t.TimerFcn = {@startMeasurement,begin, sicherheitsfaktor_aufwachen,measurementDuration_s};
t.StopFcn = @endMeasurements;
t.Period = measurementInterval_s;
t.TasksToExecute = counts;
t.ExecutionMode = 'fixedRate';
end 

function initMeasurement(mTimer,~,begin,measurementDuration_s)
secondsPerMinute = 60;
secondsPerHour = 60*secondsPerMinute;
str1 = sprintf('Starting Measurement Timer at %s', datestr(begin,'yyyy.mm.dd HH:MM:SS'));
str2 = sprintf(' For the next %d hours the Measurement will take',...
    round(mTimer.TasksToExecute*(mTimer.Period)/secondsPerHour));
str3 = sprintf(' place every %d minutes.', mTimer.Period/secondsPerMinute);
str4 = sprintf(' for a duration of %d seconds.', measurementDuration_s);
disp([str1 str2 str3 str4])
end

function startMeasurement(mTimer,~,begin,sicherheitsfaktor_aufwachen,measurementDuration_s)
    fprintf('starting µC communikation at %s\n',datestr(now,'HH:MM:SS'));
    tcp = tcpclient("192.168.137.14",23,"Timeout",mTimer.Period + 10);
    configureTerminator(tcp,13)

    while true
        a = convertCharsToStrings(char(readline(tcp)));
        if a == "start of transmission"
            break;
        end
    end
    
    dtnow = now;
    dtStart = begin + seconds(mTimer.Period*(mTimer.TasksExecuted-1)) + seconds(sicherheitsfaktor_aufwachen);
    dtEnd = dtStart+minutes(0)+seconds(measurementDuration_s);
    dtNextMeeting = begin+seconds(mTimer.Period*mTimer.TasksExecuted) - seconds(sicherheitsfaktor_aufwachen);    
    
    flush(tcp);
    writeline(tcp, "flush");
    a = datestr(dtnow,'yyyymmddHHMMSS');
    writeline(tcp, a);
    a = datestr(dtStart,'yyyymmddHHMMSS');
    writeline(tcp, a);
    a = datestr(dtEnd,'yyyymmddHHMMSS');
    writeline(tcp, a);
    a = datestr(dtNextMeeting,'yyyymmddHHMMSS');
    writeline(tcp, a);

    LinesReceived = 0;
    data = [];
    startet = 0;
    while 1
        if tcp.NumBytesAvailable > 0
            dataReceived = convertCharsToStrings(char(readline(tcp)));
            if startet == 0
                disp('starte empfang')
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
    filename = [datestr(dtStart,'yyyymmddHHMMSS') '.csv'];
    fid = fopen(filename,'wt');
    fprintf(fid,'%s\n', data{:});
    fclose(fid);
    fprintf('%d lines recieved and Saved to %s\n', LinesReceived, filename);
    mTimer.UserData = [mTimer.UserData;LinesReceived];
    fprintf('%d lines in total total\n', sum(mTimer.UserData));
    if mTimer.TasksExecuted < mTimer.TasksToExecute
        fprintf('next meeting at %s\n',datestr(dtNextMeeting,'HH:MM:SS'));
    end
    
    
    
end

function endMeasurements(mTimer,~)
    disp('al measurements taken')
    fprintf('%u LinesRecieved \n%u Lines Minimum\n%u Lines Maximum\n%u Lines Average', sum(mTimer.UserData), min(mTimer.UserData),max(mTimer.UserData),round(mean(mTimer.UserData)));
end