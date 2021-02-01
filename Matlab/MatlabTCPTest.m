if (exist('t'))
    stop(t) %end previous Timers
end
IP = "192.168.137.14";

%these values control the timings of the measurement Process
measurementDuration_s = 2;
measurementInterval_s = 55;
sicherheitsfaktor_kommunikation = 20;
counts = 100;
begin = now+minutes(0)+seconds(4);


assert(measurementInterval_s > measurementDuration_s*8, "it is not clear that there is enough time to send the data to Matlab");
assert(begin >= now+seconds(3), "begin is not far enouth in the Future");
sicherheitsfaktor_aufwachen = 7;
t = createTimer(begin,measurementInterval_s,counts,measurementDuration_s,sicherheitsfaktor_aufwachen, IP,sicherheitsfaktor_kommunikation);
startat(t,begin)



function t = createTimer(begin,measurementInterval_s,counts,measurementDuration_s,sicherheitsfaktor_aufwachen, IP,sicherheitsfaktor_kommunikation)
t = timer;
t.UserData = [];
t.StartFcn = {@initMeasurement,begin,measurementDuration_s};
t.TimerFcn = {@startMeasurement,begin, sicherheitsfaktor_aufwachen,measurementDuration_s, IP,sicherheitsfaktor_kommunikation};
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

function startMeasurement(mTimer,~,begin,sicherheitsfaktor_aufwachen,measurementDuration_s, IP,sicherheitsfaktor_kommunikation)
    fprintf("%s --> ", datestr(now,'HH:MM:SS'))
    fprintf('starting µC connection\n');
    tcp = tcpclient(IP,23,"Timeout",mTimer.Period + 10);
    configureTerminator(tcp,13)

    while 1
        a = convertCharsToStrings(char(readline(tcp)));
        if a == "start of transmission"
            fprintf("%s --> ", datestr(now,'HH:MM:SS'))
            fprintf('connection established. communication in progress\n')
            break;
        end
    end
    
    
    dtnow = now;
    dtStart = begin + seconds(mTimer.Period*(mTimer.TasksExecuted-1)) + seconds(sicherheitsfaktor_kommunikation);
    assert(dtnow<dtStart,"Start liegt vor jetzt!!!!!")
    dtEnd = dtStart+minutes(0)+seconds(measurementDuration_s);
    dtNextMeeting = begin+seconds(mTimer.Period*mTimer.TasksExecuted) - seconds(sicherheitsfaktor_aufwachen);    

    flush(tcp);
    while(tcp.NumBytesAvailable)
        read(tcp);
    end
%     writeline(tcp, "flush");
    a1 = datestr(dtnow,'yyyymmddHHMMSS');
    write(tcp, a1);
    a2 = datestr(dtStart,'yyyymmddHHMMSS');
    write(tcp, a2);
    a3 = datestr(dtEnd,'yyyymmddHHMMSS');
    write(tcp, a3);
    a4 = datestr(dtNextMeeting,'yyyymmddHHMMSS');
    write(tcp, a4);
    
    flush(tcp);
    c = readline(tcp);
    b = erase(convertCharsToStrings(char(c)),char(newline));
    if a1 ~= b
        fprintf("%s --> ", datestr(now,'HH:MM:SS'))
        fprintf("tried to send now: %s but got %s\n",a1,b)
    end
    
    c = readline(tcp);
    b = erase(convertCharsToStrings(char(c)),char(newline));
    if a2 ~= b
        fprintf("%s --> ", datestr(now,'HH:MM:SS'))
        fprintf("tried to send begin: %s but got %s\n",a2,b)
    end
   
    c = readline(tcp);
    b = erase(convertCharsToStrings(char(c)),char(newline));
    if a3 ~= b
        fprintf("%s --> ", datestr(now,'HH:MM:SS'))
        fprintf("tried to send end: %s but got %s\n",a3,b)
    end

    c = readline(tcp);
    b = erase(convertCharsToStrings(char(c)),char(newline));
    if a4 ~= b
        fprintf("%s --> ", datestr(now,'HH:MM:SS'))
        fprintf("tried to send meeting: %s but got %s\n",a4,b)
    end
    
    fprintf("%s --> ", datestr(now,'HH:MM:SS'))
    fprintf('date sync sucessfull. waiting for data. Should start at around %s at the latest\n',datestr(dtStart + seconds(5*measurementDuration_s),'HH:MM:SS'))
    startigdt = now;
    LinesReceived = 0;
    data = [];
    startet = 0;
    while 1
        if tcp.NumBytesAvailable > 0
            dataReceived = convertCharsToStrings(char(readline(tcp)));
            if startet == 0
                fprintf("%s --> ", datestr(now,'HH:MM:SS'))
                disp('receiving data....')
                startet = 1;
                receivingstartdt = now;
            end
            dataReceived = erase(dataReceived,char(newline));
            if dataReceived == "end of transmission"
                break;
            end
            data = [data dataReceived];
            LinesReceived = LinesReceived + 1;
        end 
    end
    receivingenddt = now;
    fprintf("%s --> ", datestr(now,'HH:MM:SS'))
    fprintf('date sync sucessfull. waiting for data. Should start at around %s at the latest\n',datestr(receivingenddt,'HH:MM:SS'))
    
    
    filename = ['Daten\' datestr(dtStart,'yyyymmdd_HH_MM_SS') '.csv'];
    fid = fopen(filename,'wt');
    fprintf(fid,'%s\n', data{:});
    fclose(fid);
    fprintf("%s --> ", datestr(now,'HH:MM:SS'))
    fprintf('%d lines recieved and Saved to %s\n', LinesReceived, filename);
    mTimer.UserData = [mTimer.UserData;LinesReceived startigdt receivingstartdt receivingenddt];
    fprintf("%s --> ", datestr(now,'HH:MM:SS'))
    fprintf('%d lines in total total\n', sum(mTimer.UserData(:,1)));
    flush(tcp);
    while(tcp.NumBytesAvailable)
        read(tcp);
    end
    clear tcp
    fprintf("%s --> ", datestr(now,'HH:MM:SS'))
    fprintf('cycle finished at\n')
    if mTimer.TasksExecuted < mTimer.TasksToExecute
        fprintf("%s --> ", datestr(now,'HH:MM:SS'))
        fprintf('next meeting at %s\n',datestr(dtNextMeeting+seconds(sicherheitsfaktor_aufwachen),'HH:MM:SS'));
        fprintf("%s --> ", datestr(now,'HH:MM:SS'))
        fprintf('that would be meeting %d of %d\n\n\n',mTimer.TasksExecuted+1,mTimer.TasksToExecute);
    end
    
    
    
end

function endMeasurements(mTimer,~)
    disp('al measurements taken')

    fprintf('%u LinesRecieved \n%u Lines Minimum\n%u Lines Maximum\n%u Lines Average', sum(mTimer.UserData(1,1)), min(mTimer.UserData(1,1)),max(mTimer.UserData(1,1)),round(mean(mTimer.UserData(1,1))));
    durations = [mTimer.UserData(:,4)-mTimer.UserData(:,3) mTimer.UserData(:,3)-mTimer.UserData(:,2)];

    fprintf("das csv speichern dauerte")
    datestr(durations(:,2),'HH:MM:SS')
    fprintf("das tcp speichern dauerte")
    datestr(durations(:,1),'HH:MM:SS') 

end
%     stop(t);