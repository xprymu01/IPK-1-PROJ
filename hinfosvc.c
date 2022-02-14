#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>

void error_and_exit(char *msg){
    fprintf(stderr,"%s\n",msg);
    exit(-1);
}


int get_cpu_load(){ // Inspired by https://stackoverflow.com/a/23376195  
    
    long double a[8],b[8];
    FILE *fp;

    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7]);
    fclose(fp);
    sleep(1);
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3],&b[4],&b[5],&b[6],&b[7]);
    fclose(fp);
    
    float pIdle = a[3]+a[4];
    float idle = b[3]+b[4];

    float pNonIdle = a[0]+a[1]+a[2]+a[5]+a[6]+a[7];
    float nonIdle = b[0]+b[1]+b[2]+b[5]+b[6]+b[7];

    float totald = (idle+nonIdle)-(pIdle+pNonIdle);
    float idled = idle - pIdle;

    int CPU_percentage = 100*(totald-idled)/totald;

    return CPU_percentage;
}

int main(int argc, char**argv){
    if(argc != 2)
        error_and_exit("Usage : ./hinfosvc port");
    int PORT = atoi(argv[1]);
    
    //creating server sockets 
    char server_response[1024];
    int server_socket = socket(AF_INET, SOCK_STREAM,0);
    if(server_socket == -1)
        error_and_exit("ERROR : Failed to create server socket");
    int opt = 1;
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        error_and_exit("ERROR : Failed to set server socket options");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_address,sizeof(server_address));

    if(listen(server_socket,10) == -1)
        error_and_exit("ERROR : Failed to prepare for connection");

    int addr_len = sizeof(server_address);
    int client_socket;
    while(1){
        client_socket = accept(server_socket,(struct sockaddr *)&server_address,(socklen_t *)&addr_len);
        if(client_socket == -1) 
            error_and_exit("ERROR : Failed to accept client socket");

        char buffer[1024];
        if(recv(client_socket,buffer,sizeof(buffer),0) == -1)
            error_and_exit("ERROR : Failed to recieve bytes from client socket");
        
        //parsing path from GET request
        char *start_of_path = strchr(buffer,' ') +1;
        char *start_of_protocol = strchr(start_of_path, ' ');
        char path[start_of_protocol-start_of_path];
        strncpy(path,start_of_path,start_of_protocol-start_of_path);
        path[sizeof(path)] = '\0';

        if(strcmp(path,"/hostname") == 0){
            char hostname[32];
            gethostname(hostname,sizeof(hostname));
            strcat(hostname,"\n");
            snprintf(server_response,sizeof(server_response),"HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\n%s",strlen(hostname),hostname);           
        }else if(strcmp(path,"/cpu-name") == 0){
            // parsing  cpuinfo for model name
            FILE *cpuinfo = popen("cat /proc/cpuinfo | grep 'model name' | head -n 1","r");
            char *file_line = NULL;
            size_t len = 0;
            getline(&file_line,&len,cpuinfo);
            char *cpu_name = strchr(file_line,':');        
            cpu_name = strchr(cpu_name,' ')+1;
            snprintf(server_response,sizeof(server_response),"HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\n%s",strlen(cpu_name),cpu_name);
            fclose(cpuinfo);
        }else if(strcmp(path,"/load") == 0){
            int CPU_percentage = get_cpu_load();
            int len = sizeof(CPU_percentage)+1;
            char load[len];
            snprintf(load,len,"%d%c",CPU_percentage,'%');
            strcat(load,"\n");
            load[len] = '\0';
            snprintf(server_response,sizeof(server_response),"HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\n%s",strlen(load),load);
        }else{
            snprintf(server_response,sizeof(server_response),"HTTP/1.1 404 Not Found \r\n\r\n");
        }
        if(send(client_socket,server_response,sizeof(server_response),0) == -1)
            error_and_exit("ERROR : Failed to send bytes to client socket");
        close(client_socket);
    }
    close(server_socket);
    return 0;
}