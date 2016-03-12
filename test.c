#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdlib.h>


// böyle bir fonksiyonun dosya içerisinde varolduğu önceden belirtiliyo
static void getValues(xmlNode * firstNode);
int ysetCount = 0;
static void printSpace(int times);
static void printHelpMessage();
static void createLineChart();
static void createBarChart();
static void createDynamicLineChart(xmlNode *root_node);
// argc (Argument Count) kaç tane argument olduğunu tutar
// argv (Argument Values) argumentleri string yani char arrayi olarak tutar, char arrayinin(string) arrayi oluyo *argv[]
// mesela çalıştırırken ./chartgen -i asd -a mahmut girdi
// argc = 5 olur, argv ise {"./chartgen", "-i", "asd", "-a", "mahmut"} şeklinde array halinde
struct Canvas{
  char *length, *width, *backcolor;
};

struct Axis{
  char *name, *forecolor;
};

struct Set{
  char *unit, *name, *fillcolor;
  bool showValue;
  char *values[];
};

char *title;
struct Canvas canvas;
struct Axis xaxis;
struct Axis yaxis;
struct Set xset;
struct Set ysets[10];


int main(int argc, char *argv[]) {
  int i;
  char * fileName;
  char * outputName;
  char * validationName;
  char * type;
  bool wantHelp = 0; // yardım istiyorsa bu işaretlenecek
  

  // argumentleri dolaşarak belirli komutlarımızı arayacak, mesela -i varsa sonraki argumenti inputFile olarak belirleyecek
  for(i=0; i<argc; i++){
    if(i+1!=argc && !strcmp(argv[i], "-i")){
      fileName = argv[i+1];
    }else if(i+1!=argc && !strcmp(argv[i], "-o")){
      outputName = argv[i+1];
    }else if(i+1!=argc && !strcmp(argv[i], "-v")){
      validationName = argv[i+1];
    }else if(i+1!=argc && !strcmp(argv[i], "-t")){
      type = argv[i+1];
    }else if(!strcmp(argv[i], "-h")){
      wantHelp = 1;
      break;
    }
  }
  // yardım istediyse mesajı gösterip return yapıcaz, programa devam etmeye gerek yok
  if(wantHelp){
    printHelpMessage();
    return 0;
  }

  if(!(!strcmp(type, "line") || !strcmp(type, "pie") || !strcmp(type, "bar"))){
    printf("Main Message: Invalid type. \n\n");
    printHelpMessage();
    return 0;
  }
  else if (!strcmp(type, "line"))
  {
    createLineChart(outputName);
  }
  else if (!strcmp(type, "bar"))
  {
    createBarChart(outputName);
  }
  xmlDoc *doc = xmlReadFile(fileName, NULL, 0);
  if(doc == NULL){
    printf("Main Message: Document not found. \n\n");
    printHelpMessage();
    xmlCleanupParser();
    xmlFreeDoc(doc);
    return 0;
  }
  xmlNode *root = xmlDocGetRootElement(doc);
  

  // todo: bunun içindeki valueleri kullanarak yeni svg üreticez
  getValues(root->children);

  // todo: creating svg file
  xmlDocPtr newDoc  = NULL;
  xmlNodePtr newRoot = NULL;
  

  
  xmlCleanupParser();
  xmlFreeDoc(doc);
  return 0;
}
static void createBarChart(char * outputName){
 xmlDocPtr doc = NULL;

    xmlNodePtr rootNode = NULL,node=NULL;

    doc = xmlNewDoc(BAD_CAST"1.0");
    rootNode = xmlNewNode(NULL,BAD_CAST"svg");

    xmlDocSetRootElement(doc,rootNode);
    xmlNewProp(rootNode,BAD_CAST"height",BAD_CAST"200");
    xmlNewProp(rootNode,BAD_CAST"width",BAD_CAST"200");
    int i;

    for(i=1; i<3; i++){

        node=xmlNewChild(rootNode,NULL,BAD_CAST "rect",NULL);
        xmlNewProp(node,BAD_CAST "x",BAD_CAST((i%2) ? "0" : "16"));
        xmlNewProp(node,BAD_CAST "y",BAD_CAST((i%2) ? "167" : "134"));
        xmlNewProp(node,BAD_CAST "width",BAD_CAST "16");
        xmlNewProp(node,BAD_CAST "height",BAD_CAST((i%2) ? "33" : "66"));
        xmlNewProp(node,BAD_CAST "style",BAD_CAST "fill:blue");
    }

    int j;

    for(j=1; j<3; j++){
        node=xmlNewChild(rootNode,NULL,BAD_CAST "rect",NULL);
        xmlNewProp(node,BAD_CAST "x",BAD_CAST((j%2) ? "32" : "48"));
        xmlNewProp(node,BAD_CAST "y",BAD_CAST((j%2) ? "2" : "68"));
        xmlNewProp(node,BAD_CAST "width",BAD_CAST "16");
        xmlNewProp(node,BAD_CAST "height",BAD_CAST((j%2) ? "198" : "132"));
        xmlNewProp(node,BAD_CAST "style",BAD_CAST "fill:blue");
    }

    int k;

    for(k=1; k<3; k++){

        node=xmlNewChild(rootNode,NULL,BAD_CAST "rect",NULL);
        xmlNewProp(node,BAD_CAST "x",BAD_CAST((k%2) ? "64" : "80"));
        xmlNewProp(node,BAD_CAST "y",BAD_CAST((k%2) ? "35" : "101"));
        xmlNewProp(node,BAD_CAST "width",BAD_CAST "16");
        xmlNewProp(node,BAD_CAST "height",BAD_CAST((k%2) ? "165" : "99"));
        xmlNewProp(node,BAD_CAST "style",BAD_CAST "fill:blue");
    }
        htmlSaveFileEnc(outputName, doc, "UTF-8", 1);
}
static void createDynamicLineChart(xmlNode *root_node)///
{
xmlNodePtr keepnode = NULL;
int i;
int j =0;
int range = 0;
  while(ysetCount <= j){
  for(i = 0; i< (sizeof(ysets[j].values[0])/sizeof(int)); i++){
    keepnode = xmlNewChild(root_node,NULL,BAD_CAST "line",NULL);
    xmlNewProp(keepnode,BAD_CAST "x1",range);
    

    char *str[25];
    sprintf(str,"%d",(atoi(canvas.width)-atoi(ysets[j].values[i])));
    xmlNewProp(keepnode,BAD_CAST "y1",str);
    xmlNewProp(keepnode,BAD_CAST "x2",(range+100));
    

    
    if(i != ((sizeof(ysets[j].values[0])/sizeof(int))-1)){
    sprintf(str,"%d",atoi(canvas.width)-atoi(ysets[j].values[i+1]));
    xmlNewProp(keepnode,BAD_CAST "y2",str);
      }
    }
    j++;
  }
}
static void createLineChart(char * outputName){ 

  xmlDocPtr doc = NULL;       /* document pointer */
  xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL;/* node pointers */
  xmlDtdPtr dtd = NULL;       /* DTD pointer */

  doc = xmlNewDoc(BAD_CAST "2.0");
  root_node = xmlNewNode(NULL,BAD_CAST "svg");
  xmlNewProp(root_node, BAD_CAST "xmlns", "http://www.w3.org/2000/svg");
  xmlNewProp(root_node, BAD_CAST "width", BAD_CAST canvas.width);
  xmlNewProp(root_node, BAD_CAST "height", BAD_CAST canvas.length);
  xmlDocSetRootElement(doc, root_node);

  node = xmlNewChild(root_node, NULL, BAD_CAST "line",NULL);
  node1 = xmlNewChild(root_node, NULL, BAD_CAST "line",NULL);
  node2 = xmlNewChild(root_node, NULL, BAD_CAST "polyline",NULL);
  
  
  
  xmlNewProp(node1, BAD_CAST "x1", "20");
  xmlNewProp(node1, BAD_CAST "x2", "20");
  xmlNewProp(node1, BAD_CAST "y1", "0");
  xmlNewProp(node1, BAD_CAST "y2","200");
  xmlNewProp(node1, BAD_CAST "style", "stroke:rgb(255,0,0);stroke-width:3");

  xmlNewProp(node, BAD_CAST "x1", "20");
  xmlNewProp(node, BAD_CAST "x2", "220");
  xmlNewProp(node, BAD_CAST "y1", "200");
  xmlNewProp(node, BAD_CAST "y2","200");
  xmlNewProp(node, BAD_CAST "style","stroke:rgb(255,0,0);stroke-width:3");
  createDynamicLineChart(root_node);

  htmlSaveFileEnc(outputName, doc, "UTF-8", 1);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  xmlMemoryDump();
  }
static void getValues(xmlNode * firstNode){
  xmlNode * curNode = NULL;
  xmlNode * childNode = NULL;
  xmlAttr * childAttr = NULL;
  int dataCount = 0;
  

  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE){
      if(!strcmp(curNode->name, "charttitle")){
          title = curNode->children->content;
      }else if(!strcmp(curNode->name, "canvas")){
          for(childNode = curNode->children; childNode; childNode = childNode->next){
            if(!strcmp(childNode->name, "length")){
                canvas.length = childNode->children->content;
            }else if(!strcmp(childNode->name, "width")){
                canvas.width = childNode->children->content;
            }else if(!strcmp(childNode->name, "backcolor")){
                canvas.backcolor = childNode->children->content;
            }
          }
      }else if(!strcmp(curNode->name, "Xaxis")){
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "name")){
              xaxis.name = childNode->children->content;
          }else if(!strcmp(childNode->name, "forecolor")){
              xaxis.forecolor = childNode->children->content;
          }
        }
      }else if(!strcmp(curNode->name, "Yaxis")){
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "name")){
              yaxis.name = childNode->children->content;
          }else if(!strcmp(childNode->name, "forecolor")){
              yaxis.forecolor = childNode->children->content;
          }
        }
      }else if(!strcmp(curNode->name, "Xset")){
        if(dataCount == 0){
          for(childNode = curNode->children; childNode; childNode = childNode->next){
              if(childNode->type == XML_ELEMENT_NODE){
                dataCount++;
              }
          }
        }
        int valueCount = 0;
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "xdata")){
              xset.values[valueCount++] = childNode->children->content;
          }
        }
        // CANT ITERATE ATTRIBUTES
        for(childAttr = curNode->children->properties; childAttr; childAttr = childAttr->next){
          printf("%s\n", childAttr->name);
        }
      }else if(!strcmp(curNode->name, "Yset")){
        if(dataCount == 0){
          for(childNode = curNode->children; childNode; childNode = childNode->next){
              if(childNode->type == XML_ELEMENT_NODE){
                dataCount++;
              }
          }
        }
        int valueCount = 0;
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "ydata")){
            ysets[ysetCount].values[valueCount++] = childNode->children->content;
          }
        }

        ysetCount++;
      }
    }
  }
}

static void printSpace(int times){
  int i;
  for(i=0; i<times; i++){
      printf(" ");
  }
}

static void printHelpMessage(){
  printf("NAME\n");
  printSpace(5);
  printf("chartgen - Writing SVG file from XML file\n\n");
  printSpace(5);
  printf("chartgen [-i <input fileName>]\n");
  printSpace(5);
  printf("         [-o <output fileName>]\n");
  printSpace(5);
  printf("         [-v <validation fileName>]\n");
  printSpace(5);
  printf("         [-t <type>]\n");
  printSpace(5);
  printf("         [-h]\n\n");
  printf("DESCRIPTION\n");
  printSpace(5);
  printf("         -i");
  printSpace(5);
  printf("Input file name which in XML format.\n"); 
  printSpace(5);
  printf("         -o");
  printSpace(5);
  printf("Output file name which written in SVG format.\n");
  printSpace(5);
  printf("         -v");
  printSpace(5);
  printf("????\n");
  printSpace(5);
  printf("         -t");
  printSpace(5);
  printf("Type must be 'line', 'pie' or 'bar'\n");
  printSpace(5);
  printf("         -h");
  printSpace(5);
  printf("Writing help message.\n");
}
