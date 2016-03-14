#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>

static void countData(xmlNode *firstNode);
static void countYSets(xmlNode *firstNode);
static void getValues(xmlNode *firstNode);
static void printSpace(int times);
static void printHelpMessage();
static void CreatePieChart(xmlNode *root);
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
  char **values;
};

char *title;
struct Canvas canvas;
struct Axis xaxis;
struct Axis yaxis;
struct Set xset;
struct Set ysets[10];
int ysetCount, dataCount;
char *colors[] = {"#E87162", "#5F5E5D", "#F2EEEA", "#B4B2AE", "#E24A37", "#231F20",
          "#F15E00", "#FDFF3A", "#F19C28", "#A12E33"};
char buffer[50];

int main(int argc, char *argv[]) {
  int i;
  char * fileName = NULL;
  char * outputName = NULL;
  char * validationName = NULL;
  char * type = NULL;
  bool wantHelp = 0;
  ysetCount = 0;
  dataCount = 0;

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
  countData(root->children);
  countYSets(root->children);

  getValues(root->children);

  xmlDocPtr newDoc  = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr newRoot = xmlNewNode(NULL, BAD_CAST "svg");
  sprintf(buffer, "%d", atoi(canvas.width)+100);
  xmlNewProp(newRoot, BAD_CAST "width", BAD_CAST buffer);
  sprintf(buffer, "%d", atoi(canvas.length)*ysetCount);
  xmlNewProp(newRoot, BAD_CAST "height", BAD_CAST buffer);
  xmlNewProp(newRoot, BAD_CAST "xmlns", BAD_CAST "http://www.w3.org/2000/svg");
  //HEX CHECK?
  sprintf(buffer, "background-color: #%s", canvas.backcolor);
  xmlNewProp(newRoot, BAD_CAST "style", BAD_CAST buffer);
  xmlDocSetRootElement(newDoc, newRoot);

  if(!strcmp(type, "pie")){
    CreatePieChart(newRoot);
  }else if(!strcmp(type, "bar")){
    CreateBarChart(newRoot);
  }else if(!strcmp(type, "line")){
    CreateLineChart(newRoot);
  }

  htmlSaveFileEnc(outputName, newDoc, "UTF-­8", 1);

  xmlCleanupParser();
  xmlFreeDoc(doc);
  xmlMemoryDump();
  xmlSchemaFree(schema);
  xmlSchemaCleanupTypes();
  return 0;
}

static void countData(xmlNode *firstNode){
  xmlNode *curNode = NULL;
  xmlNode *childNode = NULL;
  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE && (!strcmp(curNode->name, "Xset") || !strcmp(curNode->name, "Yset"))){
      for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(childNode->type == XML_ELEMENT_NODE){
            dataCount++;
          }
      }
      return;
    }
  }
}

static void countYSets(xmlNode *firstNode){
  xmlNode *curNode = NULL;
  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE && !strcmp(curNode->name, "Yset")){
      ysetCount++;
    }
  }
}

static void getValues(xmlNode *firstNode){
  xmlNode * curNode = NULL;
  xmlNode * childNode = NULL;
  xmlAttr * childAttr = NULL;
  int ysetIndex = 0;

  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE){
      if(!strcmp(curNode->name, "charttitle")){
          title = (char*) malloc(sizeof(char)*100);
          title = curNode->children->content;
      }else if(!strcmp(curNode->name, "canvas")){
          for(childNode = curNode->children; childNode; childNode = childNode->next){
            if(!strcmp(childNode->name, "length")){
                canvas.length = (char*) malloc(sizeof(char)*100);
                canvas.length = childNode->children->content;
            }else if(!strcmp(childNode->name, "width")){
                canvas.width = (char*) malloc(sizeof(char)*100);
                canvas.width = childNode->children->content;
            }else if(!strcmp(childNode->name, "backcolor")){
                canvas.backcolor = (char*) malloc(sizeof(char)*100);
                canvas.backcolor = childNode->children->content;
            }
          }
      }else if(!strcmp(curNode->name, "Xaxis")){
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "name")){
              xaxis.name = (char*) malloc(sizeof(char)*100);
              xaxis.name = childNode->children->content;
          }else if(!strcmp(childNode->name, "forecolor")){
              xaxis.forecolor = (char*) malloc(sizeof(char)*100);
              xaxis.forecolor = childNode->children->content;
          }
        }
      }else if(!strcmp(curNode->name, "Yaxis")){
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "name")){
              xaxis.name = (char*) malloc(sizeof(char)*100);
              yaxis.name = childNode->children->content;
          }else if(!strcmp(childNode->name, "forecolor")){
              xaxis.forecolor = (char*) malloc(sizeof(char)*100);
              yaxis.forecolor = childNode->children->content;
          }
        }
      }else if(!strcmp(curNode->name, "Xset")){
        xset.values = (char**) malloc((1+dataCount)*sizeof(char*));
        int i;
        for(i=0; i<dataCount; i++){
          xset.values[i] = (char*) malloc(sizeof(char)*100);
        }
        xset.unit = (char*) malloc(sizeof(char)*100);
        xset.fillcolor = (char*) malloc(sizeof(char)*100);
        xset.name = (char*) malloc(sizeof(char)*100);
        xset.showValue = 1;

        int valueIndex = 0;
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "xdata")){
            strcpy(xset.values[valueIndex], childNode->children->content);
            valueIndex++;
          }
        }
      }else if(!strcmp(curNode->name, "Yset")){
        ysets[ysetIndex].values = (char**) malloc((1+dataCount)*sizeof(char*));
        int i;
        for(i=0; i<dataCount; i++){
          ysets[ysetIndex].values[i] = (char*) malloc(sizeof(char)*100);
        }
        xset.unit = (char*) malloc(sizeof(char)*100);
        xset.fillcolor = (char*) malloc(sizeof(char)*100);
        xset.name = (char*) malloc(sizeof(char)*100);
        xset.showValue = 1;

        int valueIndex = 0;
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "ydata")){
            strcpy(ysets[ysetIndex].values[valueIndex], childNode->children->content);
            valueIndex++;
          }
        }

        for(childAttr = curNode->properties; childAttr; childAttr = childAttr->next){
          if(!strcmp(childAttr->name, "unit")){
            ysets[ysetIndex].unit = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "fillcolor")){
            ysets[ysetIndex].fillcolor = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "name")){
            ysets[ysetIndex].name = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "showvalue")){
            if(!strcmp(childAttr->children->content, "yes")){
              ysets[ysetIndex].showValue = 1;
            }else{
              ysets[ysetIndex].showValue = 0;
            }
          }
        }

        ysetIndex++;
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

static void CreatePieChart(xmlNode *root){
  xmlNodePtr newNode;

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST title);
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "10");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "30");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");
  int i, j;
  char number[36];

  // MATH
  for(i=0; i<ysetCount; i++){
    char **strValues = ysets[i].values;
    int intValues[dataCount];
    int sum = 0;
    for(j=0; j<dataCount; j++){
      intValues[j] = atoi(strValues[j]);
      sum += intValues[j];
    }
    int pieValues[dataCount+1];
    pieValues[0] = 0;
    for(j=1; j<=dataCount; j++){
      pieValues[j] = 360*intValues[j-1]/sum;
    }
    for(j=1; j<=dataCount; j++){
      pieValues[j] += pieValues[j-1];
    }
    pieValues[dataCount] = 360;
    bool showValues = ysets[i].showValue;
    for(j=1; j<=dataCount; j++){
      if(showValues){
        newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST ysets[i].values[j-1]);
        xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "100");
        sprintf(number, "%d", 45+j*20+150*i);
        xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
        xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "black");
      }

      newNode = xmlNewChild(root, NULL, BAD_CAST "rect", NULL);
      xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "150");
      sprintf(number, "%d", 30+j*20+150*i);
      xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
      xmlNewProp(newNode, BAD_CAST "width", BAD_CAST "20");
      xmlNewProp(newNode, BAD_CAST "height", BAD_CAST "20");
      xmlNewProp(newNode, BAD_CAST "stroke", BAD_CAST "black");
      xmlNewProp(newNode, BAD_CAST "stroke-width", BAD_CAST "1");
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST colors[j-1]);

      //sprintf(number, "%dto%d-%s", pieValues[j-1], pieValues[j], xset.values[j-1]);
      newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST xset.values[j-1]);
      xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "180");
      sprintf(number, "%d", 45+j*20+150*i);
      xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

      int a = pieValues[j-1];
      int b = pieValues[j];
      int mx = 50;
      int my = 100 + i*150;
      int x1 = mx + 30*cos(PI*a/180);
      int y1 = my + 30*sin(PI*a/180);
      int x2 = mx + 30*cos(PI*b/180);
      int y2 = my + 30*sin(PI*b/180);

      char path[255];
      sprintf(path, "M%d,%d  L%d, %d A30,30 0 0,1 %d,%d z", mx, my, x1, y1, x2, y2);

      xmlNodePtr newNode;
      newNode = xmlNewChild(root, NULL, BAD_CAST "path", NULL);
      xmlNewProp(newNode, BAD_CAST "d", BAD_CAST path);
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST colors[j-1]);

      newNode = xmlNewChild(root, NULL, BAD_CAST "text", ysets[i].name);
      sprintf(number, "%d", 150+i*150);
      xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "20");
      xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "white");
    }
  }
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
  xmlNodePtr newNode, lineNode;
  int i, j;
  char allPoints[255];
  char point[25];
  for(j = 0; j < 1; j++){ // ysetCount olarak değiştirilecek, ama üstüste yazar
    newNode = xmlNewChild(root, NULL, BAD_CAST "line",NULL);
    xmlNewProp(newNode, BAD_CAST "x1", BAD_CAST "20");
    xmlNewProp(newNode, BAD_CAST "x2", BAD_CAST "220");
    xmlNewProp(newNode, BAD_CAST "y1", BAD_CAST "200");
    xmlNewProp(newNode, BAD_CAST "y2", BAD_CAST "200");
    xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "stroke:rgb(255,0,0);stroke-width:3");

    newNode = xmlNewChild(root, NULL, BAD_CAST "line",NULL);
    xmlNewProp(newNode, BAD_CAST "x1", BAD_CAST "20");
    xmlNewProp(newNode, BAD_CAST "x2", BAD_CAST "20");
    xmlNewProp(newNode, BAD_CAST "y1", BAD_CAST "0");
    xmlNewProp(newNode, BAD_CAST "y2", BAD_CAST "200");
    xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "stroke:rgb(255,0,0);stroke-width:3");

    newNode = xmlNewChild(root, NULL, BAD_CAST "polyline",NULL);
    int maxValue = 0;
    for(i = 0; i < dataCount; i++){
      int value = atoi(ysets[j].values[i]);
      if(value > maxValue){
        maxValue = value;
      }
    }
    int coordinates[dataCount*2+4];
    coordinates[0] = 20;
    coordinates[1] = 200;
    double d;
    int x,y;
    for(i = 1; i <= dataCount; i++){
      int value = atoi(ysets[j].values[i-1]);
      d = (float) 170/dataCount;
      x = 20 + (int) ((float)i*d);
      coordinates[2*i] = x;
      d = (float) 200/maxValue;
      y = 200 - (int) ((float)value*d);
      coordinates[2*i+1] = y;
    }
    sprintf(allPoints, "");
    xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "stroke:rgb(255,0,0);stroke-width:3");
    for(i = 0; i <= dataCount; i++){
      sprintf(point, "%d,", coordinates[2*i]);
      strcat(allPoints, point);
      sprintf(point, "%d ", coordinates[2*i+1]);
      strcat(allPoints, point);
    }
    xmlNewProp(newNode, BAD_CAST "points", BAD_CAST allPoints);
  }
}
