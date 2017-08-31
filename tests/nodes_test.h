#ifndef LIBUAST_NODES_TEST_H_
#define LIBUAST_NODES_TEST_H_


#include <cstdbool>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>

extern "C" {

#include <CUnit/Basic.h>

#include "mock_node.h"
#include "roles.h"
#include "testing_tools.h"
#include "uast.h"
#include "uast_private.h"

bool EqualNodes(const Nodes *n1, const Nodes *n2) {
  if (NodesSize(n1) != NodesSize(n2)) {
    return false;
  }
  for (int i = 0; i < NodesSize(n1); i++) {
    if (NodeAt(n1, i) != NodeAt(n1, i)) {
      return false;
    }
  }
  return true;
}

void testUastNew() {
  NodeIface iface = IfaceMock();
  Uast *ctx = UastNew(iface);

  CU_ASSERT_FATAL(ctx != NULL);
  NodeIface stored_iface = UastGetIface(ctx);
  CU_ASSERT_FATAL(memcmp(&iface, &stored_iface, sizeof(NodeIface)) == 0);

  UastFree(ctx);
}

void testUastNewAlloc() {
  fail_calloc = true;
  CU_ASSERT_FATAL(UastNew(IfaceMock()) == NULL);
  fail_calloc = false;
}

void testNodesNew() {
  Nodes *nodes = NodesNew();
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(NodesAll(nodes) == NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 0);
  CU_ASSERT_FATAL(NodesCap(nodes) == 0);

  // First resize (size=10)
  CU_ASSERT_FATAL(NodesSetSize(nodes, 10) == 0);
  void *tmp1 = NodesAll(nodes);
  CU_ASSERT_FATAL(tmp1 != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 10);
  CU_ASSERT_FATAL(NodesCap(nodes) == 10);

  // Second resize (size=5)
  CU_ASSERT_FATAL(NodesSetSize(nodes, 5) == 0);
  CU_ASSERT_FATAL(tmp1 == NodesAll(nodes));
  CU_ASSERT_FATAL(NodesSize(nodes) == 5);
  CU_ASSERT_FATAL(NodesCap(nodes) == 10);

  // Second resize (size=1024)
  CU_ASSERT_FATAL(NodesSetSize(nodes, 1024) == 0);
  void *tmp2 = NodesAll(nodes);
  CU_ASSERT_FATAL(tmp2 != NULL);
  CU_ASSERT_FATAL(tmp1 != tmp2);
  CU_ASSERT_FATAL(NodesSize(nodes) == 1024);
  CU_ASSERT_FATAL(NodesCap(nodes) == 1024);

  NodesFree(nodes);
}

void testNodesNewAlloc() {
  fail_calloc = true;
  CU_ASSERT_FATAL(NodesNew() == NULL);
  fail_calloc = false;
}

void testUastFilterPointers() {
  Uast *ctx = UastNew(IfaceMock());

  Node module = Node("Module");
  Node assign_0 = Node("Assign");
  Node assign_1 = Node("Assign");
  Node assign_2 = Node("Assign");
  module.AddChild(&assign_0);
  module.AddChild(&assign_1);
  module.AddChild(&assign_2);

  Nodes *nodes = UastFilter(ctx, &module, "/Module/*");
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 3);
  CU_ASSERT_FATAL(NodeAt(nodes, 0) == &assign_0);
  CU_ASSERT_FATAL(NodeAt(nodes, 1) == &assign_1);
  CU_ASSERT_FATAL(NodeAt(nodes, 2) == &assign_2);

  NodesFree(nodes);
  UastFree(ctx);
}

void testUastFilterCount() {
  Uast *ctx = UastNew(IfaceMock());

  Node *root = TreeMock();
  // Total number of nodes
  Nodes* nodes = UastFilter(ctx, root, "//*");
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 14);
  NodesFree(nodes);

  // Total number of Modules
  nodes = UastFilter(ctx, root, "//Module");
  Nodes *nodes2 = UastFilter(ctx, root, "//*[@roleFile]");
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(nodes2 != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 1);
  CU_ASSERT_FATAL(NodeAt(nodes, 0) == root);
  CU_ASSERT_FATAL(EqualNodes(nodes, nodes2));
  NodesFree(nodes);
  NodesFree(nodes2);

  // Total number of assigns
  nodes = UastFilter(ctx, root, "//Assign");
  nodes2 = UastFilter(ctx, root, "//*[@roleAssignment]");
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(nodes2 != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 3);
  CU_ASSERT_FATAL(EqualNodes(nodes, nodes2));
  NodesFree(nodes);
  NodesFree(nodes2);

  // Total number of identifiers
  nodes = UastFilter(ctx, root, "//Identifier");
  nodes2 = UastFilter(ctx, root, "//*[@roleSimpleIdentifier]");
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(nodes2 != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 6);
  CU_ASSERT_FATAL(EqualNodes(nodes, nodes2));
  NodesFree(nodes);
  NodesFree(nodes2);

  UastFree(ctx);
}

void testUastFilterToken() {
  Uast *ctx = UastNew(IfaceMock());

  Node *root = TreeMock();

  Nodes *nodes = UastFilter(ctx, root, "/Module//*[@token='1']");
  CU_ASSERT_FATAL(nodes != NULL);
  CU_ASSERT_FATAL(NodesSize(nodes) == 1);
  Node *node = (Node *)NodeAt(nodes, 0);
  CU_ASSERT_FATAL(node->token == "1");
  CU_ASSERT_FATAL(node->internal_type == "NumLiteral");

  NodesFree(nodes);
  UastFree(ctx);
}

void testXpath() {
  NodeIface iface = IfaceMock();
  Uast *ctx = UastNew(iface);
  Node module = Node("Module");

  CU_ASSERT_FATAL(UastFilter(ctx, &module, "/Module/") == NULL);

  UastFree(ctx);
}

void _testNodeFindError() {
  NodeIface iface = IfaceMock();
  Uast *ctx = UastNew(iface);
  Node module = Node("Module");
  Node child = Node("Child");
  module.AddChild(&child);

  CU_ASSERT_FATAL(UastFilter(ctx, &module, "/Module") == NULL);

  UastFree(ctx);
}

void testXmlNewDoc() {
  fail_xmlNewDoc = true;
  _testNodeFindError();
  fail_xmlNewDoc = false;
}

void testXmlNewNode() {
  fail_xmlNewNode = true;
  _testNodeFindError();
  fail_xmlNewNode = false;
}

void testXmlNewProc() {
  fail_xmlNewProc = true;
  _testNodeFindError();
  fail_xmlNewProc = false;
}

void testXmlAddChild() {
  fail_xmlAddChild = true;
  _testNodeFindError();
  fail_xmlAddChild = false;
}

void testXmlNewContext() {
  fail_xmlXPathNewContext = true;
  _testNodeFindError();
  fail_xmlXPathNewContext = false;
}

void testNodesSetSize() {
  fail_realloc = true;
  _testNodeFindError();
  fail_realloc = false;
}
}

#endif  // LIBUAST_NODES_TEST_H_
