#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<complex.h>

#define MAXBUF 256
#define CHARMAX 32
#define NODEMAX 40
int err_val(char *str_val)
{
        int num_flag = 0,char_flag = 0,dot_flag = 0;
        int i,len;
        
        char letter;
        len = strlen(str_val);
        
        for(i = 0;i<len;i++)
        {
                letter = *(str_val + i);
                if(letter == '.')
                {       char next_letter = *(str_val + i +1);
                        
                        if ((char_flag != 0) || ((next_letter<'0') || (next_letter>'9')) || dot_flag!=0 )
                        {
                                printf("Invalid token \'value\'.Please correct the netlist\n ");
                                exit(3);
                        }
                        else
                                dot_flag++;
                }
                else if( (letter == '_') )
                {
                        printf("Invalid token \'value\'.Please correct the netlist\n ");
                        exit(3);
                }
                else if((letter >=48)&&(letter<=57))
                {
                        if (char_flag == 0)
                                num_flag++;
                        else
                        {
                                printf("Invalid token \'value\'.Please correct the netlist\n ");
                                exit(3);
                        }
                }
                
                else if ((letter>=97)&&(letter<=122))
                {
                        if (num_flag != 0)
                                char_flag++;
                        else
                        {
                                printf("Invalid token \'value\'.Please correct the netlist\n ");
                                exit(3);
                        }
                }
        }
        
        return 1;
        
}

int err_check(char *string)
{
        int len = strlen(string);
        int i = 0;
        char letter;
        for(i=0;i<len;i++)
        {       
                letter = string[i];
                if( ((letter == 9) || (letter == 10) || (letter == 32) || (letter == 95) || (letter == 46) || ( (letter>=48) 
                        && (letter <=57) ) || ( (letter>=65) && (letter<=90) ) || ( (letter>=97) && (letter<=122) ) ) );
                else
                        return i;
        }
        return len;
}
float analyse(char *string)
{
        float val;
        float mult = 1;
        int len = 0;
        
        int val_err;
        val_err = err_val(string);
        if (val_err == 1)
        {
                val = atof(string);
                len = strlen(string);
                switch(string[len-1])
                {
                        case 'k':       mult = 1000;
                                break;
                        case 'n':       mult = 1.0/10000000000.0;
                                break;
                        case 'u':       mult = 1.0/1000000.0;
                                break;
                        case 'm':       mult = 1.0/1000.0;
                                break;
                        case 'g':       if(string[len-2] == 'e' && string[len-3] == 'm')
                                        mult = 1000000;
                                        else
                                        {
                                                printf("invalid multiplier found. Please correct the netlist\n");
                                                exit(2);
                                        }
                                break;
                        default:        mult = 1;
                                break;
                }
        }
        return(val*mult);
        
}

int elem_analyse(char elem,char *indep_elem,char *v_dep_elem,char *i_dep_elem)
{
        int num_elem;
        int flag = 0;

        for(num_elem = 0;num_elem<strlen(indep_elem);num_elem++)
        {
                if (elem == indep_elem[num_elem])
                        flag = 1;
        }

        for(num_elem = 0;num_elem<strlen(v_dep_elem);num_elem++)
        {
                if (elem == v_dep_elem[num_elem])
                        flag = 2;
        }

        for(num_elem = 0;num_elem<strlen(i_dep_elem);num_elem++)
        {
                if (elem == i_dep_elem[num_elem])
                        flag = 3;
        }

        return flag;
}

int get_index(char node[32],char node_list[100][32],int num_nodes)
{
        int i;
        for (i = 0;i<num_nodes;i++)
        {
                if (strcmp(node,node_list[i]) == 0)
                        return i;
        }
}

float c_abs(complex number)
{
        float real,imag;
        float abs_val;
        real = creal(number);
        imag = cimag(number);
        
        abs_val = sqrt(real*real+imag*imag);
        return abs_val;
}

struct node
{
        char n1[CHARMAX] ;
        char n2[CHARMAX] ;
        char n3[CHARMAX] ;
        char n4[CHARMAX] ;
        char depname[CHARMAX];
        char name[CHARMAX] ;
        complex value;
        
        int vlt_src_num;

        struct node* next;
        struct node* prev;
};


typedef struct node node;

complex solver(char **argv,float frequency)
{
        FILE *fp;
        
        fp = fopen(argv[1],"r");
        
        char line[MAXBUF];
        char *word;
        char *tokbuf;
        char delim[] = " \t\n";
        int check_var;

        char indep_elem[] = {'R','L','C','V','I','r','l','c','v','i','\0'};
        char v_dep_elem[] = {'E','G','e','g','\0'};
        char i_dep_elem[] = {'F','H','f','h','\0'};

        int elem_type,num_nodes = 0;
        int vlt_src = 0;
        int cur_dep_vlt = 0;
        node *start;
        start = (node*)malloc(sizeof(node));
        
        start->next = NULL;
        start->prev = NULL;

        node *old ;
        old = (node *)malloc(sizeof(node));
        
        char clamp_nodes[2][32];
        int clamp_cnt = 0;
        while(fgets(line,MAXBUF-1,fp) != NULL)
        {
                
                if (line[0] == '#'|| line[0] == '*') ;
                
                else if(line[0] == '.')
                {
                        tokbuf = line;
                        word = strtok(tokbuf,delim);
                        if(strcmp(word,".clamp") == 0)
                        {
                                while(word!=NULL)
                                {
                                        if(clamp_cnt>1)
                                                break;
                                        tokbuf = NULL;
                                        word = strtok(tokbuf,delim);
                                        strcpy(clamp_nodes[clamp_cnt],word);
                                        clamp_cnt++;
                                }
                        }
                }
                else
                {
                        int err_code;
                        err_code = err_check(line);
                                                
                        if (err_code != strlen(line))
                        {
                                printf("Invalid character %c found.Now aborting\n",line[err_code]);
                                exit(2);
                        }
                        
                        node *temp ;
                        temp = (node*)malloc(sizeof(node));
                        
                        if(num_nodes == 0)
                                temp = start;
                        
                        tokbuf = line;
                        word = strtok(tokbuf, delim);
                        
                        strcpy(temp->name,word);
                        
                        elem_type = elem_analyse(temp->name[0],indep_elem,v_dep_elem,i_dep_elem);
                        
                        if(elem_type == 0)
                        {
                                printf("Element %s not defined\n",word);
                                exit(6);
                        }

                        if(elem_type == 1)
                        {       
                                int cnt = 0;
                                if (temp->name[0] =='V' || temp->name[0]  == 'v' )
                                {
                                        temp->vlt_src_num = vlt_src;
                                        vlt_src++;
                                }
                        
                                while(word != NULL)
                                {
                                        tokbuf = NULL;
                                        word = strtok(tokbuf,delim);
                                        
                                        if( ( word==NULL && cnt<2 ) || ( word!=NULL && cnt>2 ) )
                                        {
                                                printf("Invalid number of tokens.\n");
                                                exit(4);
                                        }
                                        
                                        switch(cnt)
                                        {
                                                case 0: strcpy(temp->n1,word);
                                                        break;
                                                case 1: strcpy(temp->n2,word);
                                                        break;
                                                case 2:         if(frequency != 0)
                                                                {
                                                                        if(temp->name[0] == 'R'||temp->name[0] == 'r')
                                                                                temp->value = analyse(word) ;
                                                                        else if(temp->name[0] == 'L'|| temp->name[0] == 'l')
                                                                                temp->value = I*frequency*analyse(word);
                                                                        else if (temp->name[0] == 'C' || temp->name[0] == 'c')
                                                                                temp->value = 1/(I*frequency*analyse(word));
                                                                        else
                                                                                temp->value =analyse(word);
                                                                }
                                                                
                                                                else
                                                                        temp->value = analyse(word);
                                                        break;
                                                default:
                                                        break;
                                                
                                        }
                                        cnt++;
                                }
                        }

                        if(elem_type == 2)
                        {
                                int cnt = 0;
                                
                                if (temp->name[0] == 'E' || temp->name[0] == 'e')
                                {       
                                        temp->vlt_src_num = vlt_src;
                                        vlt_src++;
                                }
                                

                                while(word != NULL)
                                {
                                        tokbuf = NULL;
                                        word = strtok(tokbuf,delim);
                                        
                                        if( ( word==NULL && cnt<4 ) || ( word!=NULL && cnt>4 ) )
                                        {
                                                printf("Invalid number of tokens.\n");
                                                exit(4);
                                        }

                                        switch(cnt)
                                        {
                                                case 0:         strcpy(temp->n1,word);
                                                                
                                                        break;
                                                case 1:         strcpy(temp->n2,word);
                                                        break;
                                                case 2:         strcpy(temp->n3,word);
                                                        break;
                                                case 3:         strcpy(temp->n4,word);
                                                        break;
                                                case 4:         temp->value = analyse(word) ;
                                                        break;
                                                default:
                                                        break;
                                        }
                                        cnt++;
                                }
                         
                        }

                        if(elem_type == 3)
                        {
                                int cnt = 0;
                                
                                if (temp->name[0] == 'H' || temp->name[0] == 'h')
                                {
                                        vlt_src++;
                                }

                                while(word != NULL)
                                {
                                        tokbuf = NULL;
                                        word = strtok(tokbuf,delim);
                                        
                                        if( ( word==NULL && cnt<3 ) || ( word!=NULL && cnt>3 ) )
                                        {
                                                printf("Invalid number of tokens.\n");
                                                exit(4);
                                        }

                                        switch(cnt)
                                        {
                                                case 0: strcpy(temp->n1,word);
                                                        break;
                                                case 1:         strcpy(temp->n2,word);
                                                        break;
                                                case 2: strcpy(temp->depname,word);
                                                        break;
                                                case 3: temp->value = analyse(word) ;
                                                        break;
                                                default:
                                                        break;
                                        }
                                        cnt++;
                                }
                        }
                        temp->prev = old;
                        temp->next = NULL;
                        old->next = temp;
                        old = temp;
                        
                        num_nodes++;
                        
                }
        }
        char nodes[NODEMAX][32];
        
        num_nodes = 0;
        strcpy(nodes[0],start->n1);
        strcpy(nodes[1],start->n2);
        num_nodes += 2;
        
        int i = 0,flag_n1 = 1,flag_n2 = 1;
        old = start->next;
        
        do
        {
                flag_n1 = 1;
                flag_n2 = 1;
                for (i = 0 ; i<num_nodes;i++)
                {
                        if ( strcmp(nodes[i],old->n1) == 0)
                                flag_n1 = 0;
                }
                
                if(flag_n1)
                {
                        strcpy(nodes[num_nodes],old->n1);       
                        num_nodes++;
                }
                
                for (i = 0 ; i<num_nodes;i++)
                {
                        if ( strcmp(nodes[i],old->n2) == 0)
                                flag_n2 = 0;
                }
                
                if(flag_n2)
                {
                        strcpy(nodes[num_nodes],old->n2);
                        num_nodes++;
                }
                
                old = old->next;
        }while(old != NULL);
        int dimension = num_nodes+vlt_src +1;
        complex conduct_mat[dimension-1][dimension];
        
        int j;  
        
        for(i = 0;i<dimension-1;i++)
                for(j=0;j<dimension;j++)
                        conduct_mat[i][j] = 0+0*I;
        
                        
        old = start;
        
        int eqn_cnt = 0;
        for(i = 0;i<num_nodes;i++)
        {
                old = start;
                
                int node1,node2;
                char present_node[32] ;
                strcpy(present_node,nodes[i]);
                
                if(strcmp(present_node,"0") == 0)
                {
                        node1= get_index("0",nodes,num_nodes);
                        conduct_mat[eqn_cnt][node1] += 1;
                        eqn_cnt++;
                        continue;
                }
                do
                {
                                                
                        if(strcmp(old->n1,present_node) == 0)
                        {
                                node1 = get_index(old->n1,nodes,num_nodes);
                                node2 = get_index(old->n2,nodes,num_nodes);
                                
                                if(old->name[0] == 'i' || old->name[0] == 'I')
                                        conduct_mat[eqn_cnt][dimension-1] += old->value;
                                
                                if(old->name[0] == 'r' || old->name[0] == 'R')
                                {
                                        
                                        conduct_mat[eqn_cnt][node1] += 1.0/old->value;
                                        conduct_mat[eqn_cnt][node2] -= 1.0/old->value;
                                }
                                
                                if((old->name[0] == 'l' || old->name[0] == 'L') &&(frequency != 0))
                                {
                                        
                                        conduct_mat[eqn_cnt][node1] += 1.0/old->value;
                                        conduct_mat[eqn_cnt][node2] -= 1.0/old->value;
                                }
                                
                                if((old->name[0] == 'c' || old->name[0] == 'C') && (frequency != 0))
                                {
                                        
                                        conduct_mat[eqn_cnt][node1] += 1.0/old->value;
                                        conduct_mat[eqn_cnt][node2] -= 1.0/old->value;
                                }
                                
                                if(old->name[0] == 'V' || old->name[0] == 'v')
                                        conduct_mat[eqn_cnt][num_nodes + old->vlt_src_num] += 1;
                                
                                
                                if(old->name[0] == 'G' || old->name[0] == 'g')
                                {
                                        int node3,node4;
                                        
                                        node3 = get_index(old->n3,nodes,num_nodes);
                                        node4 = get_index(old->n4,nodes,num_nodes);
                                        
                                        conduct_mat[eqn_cnt][node3] += old->value;
                                        conduct_mat[eqn_cnt][node4] -= old->value;
                                }
                                
                                if(old->name[0] == 'E' || old->name[0] == 'e')
                                        conduct_mat[eqn_cnt][num_nodes + old->vlt_src_num] += 1;
                                
                                if(old->name[0] == 'F' || old->name[0] == 'f')
                                {
                                        node *temp;
                                        temp = start;
                                        
                                        do
                                        {
                                                int cmp_chk = strcmp(old->depname,temp->name);
                                                if(cmp_chk == 0)
                                                        conduct_mat[eqn_cnt][num_nodes + temp->vlt_src_num] += old->value;
                                                
                                                temp = temp->next;
                                        }while(temp!= NULL);
                                }
        
                                
                        }
                        
                        if(strcmp(old->n2,present_node) == 0)
                        {
                                node1 = get_index(old->n1,nodes,num_nodes);
                                node2 = get_index(old->n2,nodes,num_nodes);
                                
                                if(old->name[0] == 'i' || old->name[0] == 'I')
                                        conduct_mat[eqn_cnt][dimension-1] -= old->value;
                                
                                if(old->name[0] == 'r' || old->name[0] == 'R')
                                {
                                        
                                        conduct_mat[eqn_cnt][node1] -= 1.0/old->value;
                                        conduct_mat[eqn_cnt][node2] += 1.0/old->value;
                                }
                                
                                if((old->name[0] == 'l' || old->name[0] == 'L') && (frequency != 0))
                                {
                                        
                                        conduct_mat[eqn_cnt][node1] -= 1.0/old->value;
                                        conduct_mat[eqn_cnt][node2] += 1.0/old->value;
                                }
                                
                                if(old->name[0] == 'c' || old->name[0] == 'C' && (frequency != 0))
                                {
                                        
                                        conduct_mat[eqn_cnt][node1] -= 1.0/old->value;
                                        conduct_mat[eqn_cnt][node2] += 1.0/old->value;
                                }
                                
                                if(old->name[0] == 'V' || old->name[0] == 'v')
                                        conduct_mat[eqn_cnt][num_nodes + old->vlt_src_num] -= 1;
                                
                                
                                if(old->name[0] == 'G' || old->name[0] == 'g')
                                {
                                        int node3,node4;
                                        
                                        node3 = get_index(old->n3,nodes,num_nodes);
                                        node4 = get_index(old->n4,nodes,num_nodes);
                                        
                                        conduct_mat[eqn_cnt][node3] -= old->value;
                                        conduct_mat[eqn_cnt][node4] += old->value;
                                }
                                
                                if(old->name[0] == 'E' || old->name[0] == 'e')
                                        conduct_mat[eqn_cnt][num_nodes + old->vlt_src_num] -= 1;
                                
                                if(old->name[0] == 'F' || old->name[0] == 'f')
                                {
                                        node *temp;
                                        temp = start;
                                        
                                        do
                                        {
                                                int cmp_chk = strcmp(old->depname,temp->name);
                                                if(cmp_chk == 0)
                                                        conduct_mat[eqn_cnt][num_nodes + temp->vlt_src_num] -= old->value;
                                                
                                                temp = temp->next;
                                        }while(temp!= NULL);
                                }
                                
                        }
                        
                        old = old->next;
                        
                }while(old != NULL);
                
                eqn_cnt++;
        }
        int vlt_src_cnt = 0;
        old = start;
                
        do
        {

                if(old->name[0] == 'V' || old->name[0] == 'v')
                {
                        int node1 = get_index(old->n1,nodes,num_nodes);
                        int node2 = get_index(old->n2,nodes,num_nodes);

                        conduct_mat[num_nodes + vlt_src_cnt][node1] += 1;
                        conduct_mat[num_nodes + vlt_src_cnt][node2] -= 1;

                        conduct_mat[num_nodes + vlt_src_cnt][dimension-1] += old->value;
                        vlt_src_cnt++;
                }

                if(old->name[0] == 'E' || old->name[0] == 'e')
                {
                        int node1 = get_index(old->n1,nodes,num_nodes);
                        int node2 = get_index(old->n2,nodes,num_nodes);
                        int node3 = get_index(old->n3,nodes,num_nodes);
                        int node4 = get_index(old->n4,nodes,num_nodes);

                        conduct_mat[num_nodes + vlt_src_cnt][node1] += 1;
                        conduct_mat[num_nodes + vlt_src_cnt][node2] -= 1;
                        conduct_mat[num_nodes + vlt_src_cnt][node3] -= old->value;
                        conduct_mat[num_nodes + vlt_src_cnt][node4] += old->value;

                        vlt_src_cnt++;
                        
                }

                if(old->name[0] == 'H' || old->name[0] == 'h')
                {
                        int node1 = get_index(old->n1,nodes,num_nodes);
                        int node2 = get_index(old->n2,nodes,num_nodes);

                        node *temp;
                        temp = start;

                        do
                        {

                                int cmp_chk = strcmp(temp->name,old->depname);
                                if (cmp_chk == 0)
                                {
                                        conduct_mat[num_nodes + vlt_src_cnt][node1] += 1+0*I;
                                        conduct_mat[num_nodes + vlt_src_cnt][node2] -= 1+0*I;

                                        conduct_mat[num_nodes + vlt_src_cnt][num_nodes + temp->vlt_src_num] += old->value;
                                }
                                        
                                temp = temp->next;
                        }while(temp != NULL);
                }
                old = old->next;
        }while(old != NULL);
        
                
        int k = 0;
        for(i=0;i<dimension-1;i++)
        {
                if(conduct_mat[i][i] == (0+0*I))
                {
                        for(j = 0;j<dimension-1;j++)
                        {
                                if(conduct_mat[j][i] !=0+0*I)
                                        break;
                        }
                
                
                        for(k = 0;k<dimension;k++)
                        {
                                conduct_mat[i][k] +=conduct_mat[j][k];
                        }
                }
        }
        for(i=1;i<dimension-1;i++)
        {
                for(j=0;j<i;j++)
                {
                        if(conduct_mat[i][j] == 0)
                                continue;
                        complex mult_factor = conduct_mat[i][j]/conduct_mat[j][j];
                        
                        for(k=j;k<dimension;k++)
                        {
                                conduct_mat[i][k]  -= mult_factor*conduct_mat[j][k] ;
                        }
                                
                }
        }
        
        for(i = 0;i<dimension-1;i++)
        {
                if(conduct_mat[i][i] == 0)
                {
                        printf("Indeterminate system.Analysis will stop.\n");
                        exit(8);
                }
        }
        
        complex solution[dimension-1];
        for(i = 0;i<dimension-1;i++)
                solution[i] = 0+0*I;
        
        solution[dimension-2] = (conduct_mat[dimension-2][dimension-1]/conduct_mat[dimension-2][dimension-2]);
        
        for(i=dimension-3;i>=0;i--)
        {
                complex subtractor = 0+0*I;
                for (j=dimension-2;j>i;j--)
                {
                        subtractor += conduct_mat[i][j]*solution[j];
                }
                solution[i] = ((conduct_mat[i][dimension-1]) - subtractor)/conduct_mat[i][i];
        }
        
        complex vlt1,vlt2,vlt_diff;
        vlt1 = solution[get_index(clamp_nodes[0],nodes,num_nodes)];
        vlt2 = solution[get_index(clamp_nodes[1],nodes,num_nodes)];
        vlt_diff = vlt2-vlt1;
                
        return vlt_diff;
        
        fclose(fp);

}

int main(int argc,char **argv)
{
        float frequency;
        float strt_freq,end_freq,step_size;
        int steps;
        complex voltage = 0+0*I;

        FILE *data;
        
        printf("Program: Myspice\n");

        data = fopen("plot_data.dat","w");
        printf("opened netlist: %s\n",argv[1]);
        
        if(argc<=1)
        {
                printf("Control file missing.\n");
                printf("Usage: ./executable controlfile\n");
                printf("Now aborting\n");
                exit(1);
        }
        
        else if(argc == 2)
        {
                printf("DC analysis chosen\n");
                voltage = solver(argv,0);
                printf("voltage at requested node is %f\n",c_abs(voltage));
        }
        else if(argc == 3) 
        {
                printf("Single frequency analysis at %s\n",argv[2]);
                frequency = atof(argv[2]);
                voltage = solver(argv,frequency);
                printf("Voltage at requested node at %f frequency is %f",frequency,c_abs(voltage));
        }
        
        else if(argc == 5)
        {
                printf("Frequency sweep.\n");
                
                strt_freq = atof(argv[2]);
                end_freq = atof(argv[3]);
                steps = atof(argv[4]);
                step_size = (end_freq-strt_freq)/steps;
                
                int i =0;
                for (i =0;i<steps;i++)
                {
                        frequency = strt_freq+step_size*i;
                        voltage = solver(argv,frequency);
                        fprintf(data,"%f %f\n",frequency,c_abs(voltage));
                }
                
                fclose(data);
                
                printf("data file created as plot_data.dat\n");
                printf("thank you for using myspice.use 'python plot.py plot_data.dat'for getting a plot of the frequency sweep.\n\n");
                
        }
        
}
