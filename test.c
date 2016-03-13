#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>
#include <math.h>

static void getValues(xmlNode *firstNode);
static void printSpace(int times);
static void printHelpMessage();
static void CreatePieChart(xmlNode *root);
static void pathD(xmlNode *root, int a, int b);
static void CreateBarChart(xmlNode *root);
static void CreateLineChart(xmlNode *root);

#define PI 3.14159265359

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
int ysetCount, dataCount;

int main(int argc, char *argv[]) {
  int i;
  char * fileName = NULL;
  char * outputName = NULL;
  char * validationName = NULL;
  char * type = NULL;
  bool wantHelp = 0;

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
  if(wantHelp){
    printHelpMessage();
    return 0;
  }
  if(fileName == NULL){
    printf("Main Message: There is no input file. \n\n");
    printHelpMessage();
    return 0;
  }else if(validationName == NULL){
    printf("Main Message: There is no validation file. \n\n");
    printHelpMessage();
    return 0;
  }else if(!(!strcmp(type, "line") || !strcmp(type, "pie") || !strcmp(type, "bar"))){
    printf("Main Message: Invalid typbuffer. \n\n");
    printHelpMessage();
    return 0;
  }

  xmlDoc *doc = xmlReadFile(fileName, NULL, 0);
  if(doc == NULL){
    printf("Main Message: Document not found. \n\n");
    printHelpMessage();
    return 0;
  }
  xmlNode *root = xmlDocGetRootElement(doc);

  xmlLineNumbersDefault(1);
  xmlSchemaParserCtxtPtr context = xmlSchemaNewParserCtxt(validationName);
  xmlSchemaPtr schema = xmlSchemaParse(context);
  xmlSchemaValidCtxtPtr validContext = xmlSchemaNewValidCtxt(schema);
  int validRes = xmlSchemaValidateDoc(validContext, doc);
  if(validRes != 0){
    printf("Main Message: XML error. \n\n");
    printHelpMessage();
    xmlCleanupParser();
    xmlFreeDoc(doc);
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    return 0;
  }

  // todo: bunun içindeki valueleri kullanarak yeni svg üreticez
  getValues(root->children);

  // todo: creating svg file
  xmlDocPtr newDoc  = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr newRoot = xmlNewNode(NULL, BAD_CAST "svg");
  xmlNewProp(newRoot, BAD_CAST "width", BAD_CAST "500");
  xmlNewProp(newRoot, BAD_CAST "height", BAD_CAST "500");
  xmlNewProp(newRoot, BAD_CAST "xmlns", BAD_CAST "http://www.w3.org/2000/svg");
  xmlNewProp(newRoot, BAD_CAST "style", BAD_CAST "background-color: blue;");
  xmlDocSetRootElement(newDoc, newRoot);

  if(!strcmp(type, "pie")){
    CreatePieChart(newRoot);
      pathD(newRoot, 90, 150);
        pathD(newRoot, 70, 90);
  }else if(!strcmp(type, "bar")){
    CreateBarChart(newRoot);
  }else if(!strcmp(type, "line")){
    CreateLineChart(newRoot);
  }

  htmlSaveFileEnc(outputName, newDoc, "UTF­8", 1);

  xmlCleanupParser();
  xmlFreeDoc(doc);
  xmlMemoryDump();
  xmlSchemaFree(schema);
  xmlSchemaCleanupTypes();
  return 0;
}

static void getValues(xmlNode *firstNode){
  xmlNode * curNode = NULL;
  xmlNode * childNode = NULL;
  xmlAttr * childAttr = NULL;

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

        for(childAttr = curNode->properties; childAttr; childAttr = childAttr->next){
          if(!strcmp(childAttr->name, "unit")){
            ysets[ysetCount].unit = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "fillcolor")){
            ysets[ysetCount].fillcolor = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "name")){
            ysets[ysetCount].name = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "fillcolor")){
            if(!strcmp(childAttr->children->content, "yes")){
              ysets[ysetCount].showValue = 1;
            }else{
              ysets[ysetCount].showValue = 0;
            }
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

static void pathD(xmlNode *newRoot, int a, int b){
  int x1 = 200 + 180*cos(PI*a/180);
  int y1 = 200 + 180*sin(PI*a/180);

  int x2 = 200 + 180*cos(PI*b/180);
  int y2 = 200 + 180*sin(PI*b/180);

  char number[36];
  char path[255];
  strcpy(path, "M200,200  L");
  sprintf(number, "%d", x1);
  strcat(path, number);
  strcat(path, ",");
  sprintf(number, "%d", y1);
  strcat(path, number);
  strcat(path, "  A180,180 0 0,1 ");
  sprintf(number, "%d", x2);
  strcat(path, number);
  strcat(path, ",");
  sprintf(number, "%d", y2);
  strcat(path, number);
  strcat(path, " z");

  printf("%s\n", path);

    xmlNodePtr newNode;
  newNode = xmlNewChild(newRoot, NULL, BAD_CAST "path", NULL);
  xmlNewProp(newNode, BAD_CAST "d", BAD_CAST path);
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");
}

static void CreatePieChart(xmlNode *root){
  xmlNodePtr newNode;

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST title);
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "10");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "30");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

  newNode = xmlNewChild(root, NULL, BAD_CAST "circle", NULL);
  xmlNewProp(newNode, BAD_CAST "cx", BAD_CAST "50");
  xmlNewProp(newNode, BAD_CAST "cy", BAD_CAST "90");
  xmlNewProp(newNode, BAD_CAST "r", BAD_CAST "30");
  xmlNewProp(newNode, BAD_CAST "stroke", BAD_CAST "black");
  xmlNewProp(newNode, BAD_CAST "stroke-width", BAD_CAST "1");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST xset.name);
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "50");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "70");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "white");

  newNode = xmlNewChild(root, NULL, BAD_CAST "rect", NULL);
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "100");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "80");
  xmlNewProp(newNode, BAD_CAST "width", BAD_CAST "20");
  xmlNewProp(newNode, BAD_CAST "height", BAD_CAST "20");
  xmlNewProp(newNode, BAD_CAST "stroke", BAD_CAST "black");
  xmlNewProp(newNode, BAD_CAST "stroke-width", BAD_CAST "1");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST "Name #1");
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "130");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "95");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST "City #1");
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "30");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "140");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "white");

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST "#1");
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "40");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "95");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "blue");
  xmlNewProp(newNode, BAD_CAST "transform", BAD_CAST "rotate(0)");
}

static void CreateBarChart(xmlNode *root){
    xmlNodePtr newNode;
    int i;
    for(i=1; i<3; i++){
        newNode=xmlNewChild(root,NULL,BAD_CAST "rect",NULL);
        xmlNewProp(newNode,BAD_CAST "x",BAD_CAST((i%2) ? "0" : "16"));
        xmlNewProp(newNode,BAD_CAST "y",BAD_CAST((i%2) ? "167" : "134"));
        xmlNewProp(newNode,BAD_CAST "width",BAD_CAST "16");
        xmlNewProp(newNode,BAD_CAST "height",BAD_CAST((i%2) ? "33" : "66"));
        xmlNewProp(newNode,BAD_CAST "style",BAD_CAST "fill:blue");
    }
    for(i=1; i<3; i++){
        newNode=xmlNewChild(root,NULL,BAD_CAST "rect",NULL);
        xmlNewProp(newNode,BAD_CAST "x",BAD_CAST((i%2) ? "32" : "48"));
        xmlNewProp(newNode,BAD_CAST "y",BAD_CAST((i%2) ? "2" : "68"));
        xmlNewProp(newNode,BAD_CAST "width",BAD_CAST "16");
        xmlNewProp(newNode,BAD_CAST "height",BAD_CAST((i%2) ? "198" : "132"));
        xmlNewProp(newNode,BAD_CAST "style",BAD_CAST "fill:blue");
    }
    for(i=1; i<3; i++){
        newNode=xmlNewChild(root,NULL,BAD_CAST "rect",NULL);
        xmlNewProp(newNode,BAD_CAST "x",BAD_CAST((i%2) ? "64" : "80"));
        xmlNewProp(newNode,BAD_CAST "y",BAD_CAST((i%2) ? "35" : "101"));
        xmlNewProp(newNode,BAD_CAST "width",BAD_CAST "16");
        xmlNewProp(newNode,BAD_CAST "height",BAD_CAST((i%2) ? "165" : "99"));
        xmlNewProp(newNode,BAD_CAST "style",BAD_CAST "fill:blue");
    }
}

static void CreateLineChart(xmlNode *root){
  xmlNodePtr newNode;
  newNode = xmlNewChild(root, NULL, BAD_CAST "line",NULL);
  xmlNewProp(newNode, BAD_CAST "x1", "20");
  xmlNewProp(newNode, BAD_CAST "x2", "220");
  xmlNewProp(newNode, BAD_CAST "y1", "200");
  xmlNewProp(newNode, BAD_CAST "y2","200");
  xmlNewProp(newNode, BAD_CAST "style","stroke:rgb(255,0,0);stroke-width:3");

  newNode = xmlNewChild(root, NULL, BAD_CAST "line",NULL);
  xmlNewProp(newNode, BAD_CAST "x1", "20");
  xmlNewProp(newNode, BAD_CAST "x2", "20");
  xmlNewProp(newNode, BAD_CAST "y1", "0");
  xmlNewProp(newNode, BAD_CAST "y2","200");
  xmlNewProp(newNode, BAD_CAST "style", "stroke:rgb(255,0,0);stroke-width:3");

  xmlNewChild(root, NULL, BAD_CAST "polyline",NULL); // illa bi değişkene eşitlemek zorunda değilsin

  int i, j;
  int range = 0;
  char str[25];
  for(i = 0, j = 0; j < ysetCount && i < dataCount; i++, j++){
    newNode = xmlNewChild(root,NULL,BAD_CAST "line",NULL);
    sprintf(str, "%d", range);
    xmlNewProp(newNode, BAD_CAST "x1", BAD_CAST str);
    sprintf(str,"%d",(atoi(canvas.width)-atoi(ysets[j].values[i])));
    xmlNewProp(newNode, BAD_CAST "y1", BAD_CAST str);
    sprintf(str, "%d", (range+100));
    xmlNewProp(newNode, BAD_CAST "x2", BAD_CAST str);

    if(i != ((sizeof(ysets[j].values[0])/sizeof(int))-1)){
      sprintf(str,"%d",atoi(canvas.width)-atoi(ysets[j].values[i+1]));
      xmlNewProp(newNode, BAD_CAST "y2", BAD_CAST str);
    }
  }
}
