/*
    NAME: Jaxchan   
    GITHUB: Jaxchan25
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "getopt.h"
#include "time.h"
#include "cachelab.h"

int hits = 0 ,misses = 0 ,evictions = 0; 
int DEBUG = 0;

typedef  struct
{
  int valid;//有效位
  unsigned long tag; //标记位
    //这里是否需要存高速缓存块呢
  clock_t time_stamp;

}cache_line;

/*
  @brief: 初始化二维数组，动态分配内存
  @param:S 多少个组
  @param:E 每组有多少个行
*/
cache_line** initiate(int S,int E){
  cache_line** cache;
  cache = (cache_line** ) malloc(sizeof(cache_line) * S); 
    for(int i=0;i<S;i++)
    {
      int size = sizeof(cache_line) * E;
      cache[i] = (cache_line* ) malloc(size);
      memset(cache[i], 0, size);
    }
  return cache;
}

/*
  清理二维数组
*/
int clean(cache_line** c, int S){
  /* cache_line 二维数组的清理工作 */
  int i;
  for(i=0;i<S;i++)
    {
      free(c[i]);
    }
  free(c);
  return 0;
}


int isHit
(
  cache_line* cache_line_group,
  int E,
  unsigned long t
){

  //DEBUG
  if(DEBUG){
    printf("\n DEBUG IN isHit \t");
    for(int i =0;i<E;i++){
        printf("\n INDEX: %d ",i);
        printf(" FOUND USING TAG: %lx \t",cache_line_group[i].tag);
    }
    printf("\nEND DEBUG IN isHit\n");
  }

  for(int i =0;i<E;i++){
    //hit：有效位 =1 ,且标记位对得上
    if (cache_line_group[i].valid==1&&cache_line_group[i].tag == t){
      cache_line_group[i].time_stamp = clock();
      hits+=1;
      printf(" FOUND USING TAG: %lx \t",t);
      return 1;
    }
  }
  return 0;
}

int putInCache(
  cache_line* cache_line_group,
  int E,
  unsigned long t
){

  //直接可以放在空位。
  for(int i =0;i<E;i++){
    //hit：有效位 =1 ,且标记位对得上
    if (cache_line_group[i].valid==0){
      cache_line_group[i].valid = 1;
      cache_line_group[i].tag = t;
      cache_line_group[i].time_stamp = clock();
      printf(" PLACE IN BLANK USING TAG: %lx \t",t);
      printf(" UPDATE TAG: %lx\t",cache_line_group[i].tag);
     return 0;
    }
  }

  //需要进行evictions
  //搜索
  int LRU_index = 0;
  clock_t LRU_time_stamp = cache_line_group[0].time_stamp; //时间越久，数值越少
  for(int i =0;i<E;i++){
    if (cache_line_group[i].time_stamp<LRU_time_stamp){
      LRU_index = i;
      LRU_time_stamp = cache_line_group[i].time_stamp;
    }
  }
  //进行
  cache_line_group[LRU_index].tag = t;
  cache_line_group[LRU_index].time_stamp = clock();
  evictions+=1;
  printf(" PLACE BY EVICTIONS USING TAG: %lx\t",t);
  printf(" UPDATE TAG: %lx\t UPDATE INDEX: %d\t",
    cache_line_group[LRU_index].tag,LRU_index);
  return 1;
}

void print_verbose(char* pre, char type, int hit_miss, int eviction){
  /* 命令行带 -v 的话的详细数据输出函数 */
  char* h = hit_miss?" hit":" miss";
  char* e = eviction?" eviction":"";
  char* format;
  if(type == 'M')
    { 
      //如果是M模式，最后一定是hit
      format = "%s%s%s\n";
      strcat(pre, format);
      printf(pre, h, e, " hit");
    }
  else
    { 
      format = "%s%s\n";
      strcat(pre, format);
      printf(pre, h, e);
    }
}


/*
  @brief: 访存核心部分。处理各个指令，判断是否hit\miss\evictions
  @param: cache 缓存，二维数组表示
  @param: type，命令指令类型，分别为L\S\M
  @param: address，命令访问地址，64位长度hex
  @param: size,命令访问块的大小，单位字节。这里不影响解题，只是输出就好。
  @param: b_mask s_mask t_mask都是cache本身的超参数，做成了mask是方便计算。
  @param: E,b 是cache本身的超参数。
  @param: hits misses evictions 击中\丢失\替换的累计值
*/
void handleCore(
  cache_line** cache,
  char type,
  unsigned long address,
  int size,
  unsigned long b_mask,
  unsigned long s_mask,
  unsigned long t_mask,
  int E,
  int b,
  int verbose
){
  //对地址解析
  unsigned long t = address&t_mask; 
  unsigned long s =  address&s_mask;

  //取出组
  unsigned long s_id = s>>b;
  cache_line* cache_line_group = cache[s_id];
  printf("组号：%lu ,tag: %lx    \t",s_id,t);

  //查看是否hit，如果hit直接更新时钟。
  int flag_hit = isHit(cache_line_group,E,t);

  //如果没有hit，就要evict或放入空位置
  int flag_evict = 0;
  if(flag_hit==0){
    misses+=1;
    flag_evict = putInCache(cache_line_group,E,t);
  }

  //如果指令是M，需要额外+1个hit
  if(type=='M'){
    hits+=1;
  }

  //输出一下
  if(verbose)
  { char pre[20];
    sprintf(pre, "%c %lx,%d", type, address, size);
    print_verbose(pre, type, flag_hit, flag_evict);
  }


}




int main(int argc, char * argv[])
{      
  //命令行解析部分
  int verbose = 0;
  int s,E,b;
  char* t;
  int ch;
  while ((ch = getopt(argc, argv, "s:E:b:t:v")) != -1){
        switch (ch) {
          case 's':
              s = atoi(optarg);
              break;
          case 'E':
              E = atoi(optarg);
              break;
          case 'b':
              b = atoi(optarg);
              break;
          case 't':
              t = optarg;
              break;
          case 'v':
              verbose = 1;
              break;
          default:
              exit(-1);
        }
  }
  printf("输入命令行：s:%d E:%d b:%d verbose:%d t:%s\n",s,E,b,verbose,t);
  int S = 1<<s;
  //int B = 1<<b;
  /*
    初始化二维数组
  */
  cache_line** cache;
  cache = initiate(S,E);


  //读取文件
  FILE* fp = fopen(t, "r");
  if(fp == NULL)
  {
    printf("%s: No such file or directory\n", t);
    exit(1);
  }
  /*
      定义数据结构；读入数据；拟真
  */

  unsigned long b_mask = 1<<b; 
  b_mask -=1; //0000...0 00001111

  unsigned long s_mask = 1<<(s+b) ;
  s_mask = (s_mask-1)^ b_mask;//0000...0 11110000

  unsigned long t_mask = 1<<(s+b);
  t_mask = (t_mask - 1 )^(~0);//1111...1 00000000

  printf("b_mask: %lx ",b_mask);
  printf("s_mask: %lx ",s_mask);
  printf("t_mask: %lx \n",t_mask);

  char type;
  int size;
  unsigned long address;
  while(fscanf(fp, " %c %lx,%d", &type, &address, &size) != EOF)
  {
    if(type == 'I'){
        continue;
    }
    else{
        //接口进入
        handleCore(cache,type,address,size,b_mask,s_mask,t_mask,E,b,verbose);

    }
  }

  printSummary(hits, misses, evictions);
  clean(cache,S);
  fclose(fp);
  return 0;
}