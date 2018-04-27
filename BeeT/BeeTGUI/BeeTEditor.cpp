#include "BeeTEditor.h"
#include "BeeTGui.h"
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "FileSystem.h"
#include "ThirdParty/NodeEditor/Include/NodeEditor.h"
#include "BehaviorTree.h"
#include "BTNode.h"
#include "BTLink.h"
#include "ItemList.h"
#include "Blackboard.h"

#include <vector>

namespace ne = ax::NodeEditor;
using namespace std;

BeeTEditor::BeeTEditor()
{
}

BeeTEditor::~BeeTEditor()
{
}

bool BeeTEditor::Init()
{
	bt = new BehaviorTree();
	bb = new Blackboard();
	ne::CenterNodeOnScreen(0); // Root node has always id = 0. Careful! It may not work in the future.
	widgetItemList = new ItemList();
	widgetBBList = new ItemList();
	InitBBListCategories();
	return true;
}

bool BeeTEditor::Update()
{
	g_app->window->GetWindowSize(screenWidth, screenHeight);
	editorSize.x = (float) screenWidth;
	editorSize.y = screenHeight - (ImGui::GetCursorPosY() - ImGui::GetCursorPosX());

	BlackBoardWindow();
	Editor();
	UpdateSelection();
	Inspector();

	return true;
}

bool BeeTEditor::CleanUp()
{
	delete bb;
	delete bt;
	delete widgetItemList;
	delete bbVarListObj;
	delete widgetBBList;
	return true;
}

void BeeTEditor::Serialize(const char* filename) const
{
	// In progress: Now only save one BT. In the future choose one of the opened BTs to save or save them all.
	char* buffer = nullptr;
	int size = bt->Serialize(&buffer);
	if (size == 0)
	{
		LOGE("Behavior Tree was not saved. An error occurred during serialization.");
	}
	else
	{
		unsigned int ret = g_app->fileSystem->Save(filename, buffer, size);
		if(ret == 0)
			LOGE("Behavior Tree was not saved. An error occurred while writting data on a file.")
	}

	if (buffer)
		delete buffer;
}

void BeeTEditor::Load(const char * path)
{
	char* buffer = nullptr;
	unsigned int size = g_app->fileSystem->Load(path, &buffer);
	if (size > 0)
	{
		Data btData(buffer);

		NewBehaviorTree(&btData); // For now the current BT is destroyed and replaced.
	}
	else
	{
		LOGE("Could not open the file %s", path);
	}

	if (buffer)
		delete buffer;
}

void BeeTEditor::NewBehaviorTree(Data* data)
{
	g_app->beetGui->ResetNodeEditorContext();
	if (bb)
		delete bb;
	if (bt)
		delete bt;
	if (data)
	{
		bt = new BehaviorTree(*data);
		//TODO: BB DATA
	}
	else
	{
		bt = new BehaviorTree();
		bb = new Blackboard();
	}
	ne::CenterNodeOnScreen(0); // Root node has always id = 0. Careful! It may not work in the future.
}

void BeeTEditor::CallBackAddNode(void * obj, const std::string & category, const std::string & item, int additionalData)
{
	BeeTEditor* editor = ((BeeTEditor*)obj);
	if (editor && g_app && g_app->beetGui)
	{
		int id = g_app->beetGui->btNodeTypes->GetNodeTypeId(category, item);
		if (id != -1)
		{
			ImVec2 pos = ne::ScreenToCanvas(ImGui::GetMousePos());
			int nodeId = editor->bt->AddNode(pos.x, pos.y, id);
			editor->selectedNodeId = nodeId;
			ne::ClearSelection();
			ne::SelectNode(nodeId);
			editor->nodeAddedFlag = true;
		}
	}
}

void BeeTEditor::CallBackBBVarType(void * obj, const std::string & category, const std::string & item, int additionalData)
{
	BeeTEditor* editor = (BeeTEditor*)obj;
	if (editor)
	{
		BBVar* var = editor->bb->variables[additionalData];
		if (var)
		{
			map<string, BBVarType>::iterator it = editor->bbVarTypeConversor.find(item);
			if (it != editor->bbVarTypeConversor.end())
			{
				var->type = it->second;
				editor->bb->SetLastTypeUsed(it->second);
			}
		}
	}
}

void BeeTEditor::BlackBoardWindow()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetCursorPosY() - ImGui::GetCursorPosX()));
	ImGui::SetNextWindowSize(ImVec2(editorSize.x * blackboardSize.x, editorSize.y * blackboardSize.y));
	ImGui::Begin("BlackBoard", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
	
	if (widgetBBList->IsVisible())
		widgetBBList->Draw();

	for (int i = 0; i < bb->variables.size(); ++i)
	{
		BBVar* bbvar = bb->variables[i];
		// Var TYPE
		switch (bbvar->type)
		{
		case BV_BOOL:
			ImGui::Text("bool   ");
			break;
		case BV_INT:
			ImGui::Text("int    ");
			break;
		case BV_FLOAT:
			ImGui::Text("float  ");
			break;
		case BV_STRING:
			ImGui::Text("string ");
			break;
		default:
			ImGui::Text("Error  ");
			break;
		}
		
		if (ImGui::IsItemClicked())
		{
			widgetBBList->SetWidgetPosition(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			widgetBBList->SetSelFunctionCallback(BeeTEditor::CallBackBBVarType, this, i);
			widgetBBList->SetVisible(true, bbVarListObj);
		}

		ImGui::SameLine();

		if (bbvarSelected == i) // Edit name
		{
			char varNameTmp[_MAX_PATH];
			strcpy_s(varNameTmp, _MAX_PATH, bbvar->name.data());
			ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue;
			ImGui::PushID(i);
			if (bbvarSetFocus)	
				ImGui::SetKeyboardFocusHere(0);
		
			if (ImGui::InputText("###", varNameTmp, _MAX_PATH, inputFlags))
			{
				bbvar->name = varNameTmp;
				bbvarSelected = -1;
			}	
			if (ImGui::IsItemActive() == false && bbvarSetFocus == false)
			{
				bbvarSelected = -1;
			}
			if (bbvarSetFocus)
				bbvarSetFocus = false;
			ImGui::PopID();
		}
		else // Display name as selectable
		{
			if (ImGui::Selectable(bbvar->name.data(), false, ImGuiSelectableFlags_::ImGuiSelectableFlags_AllowDoubleClick))
			{
				if (ImGui::IsMouseDoubleClicked(0))
				{
					bbvarSelected = i;
					bbvarSetFocus = true;
				}
			}
		}	
	}

	if (ImGui::Button("Add New"))
	{
		bb->CreateDummyVar();
		bbvarSelected = bb->variables.size() - 1;
		bbvarSetFocus = true;
	}

	ImGui::End();
}

void BeeTEditor::Editor()
{
	ImGui::SetNextWindowPos(ImVec2(screenWidth * blackboardSize.x, ImGui::GetCursorPosY() - ImGui::GetCursorPosX())); // The Y component substracts the cursorX position because imgui by default has margins
	ImGui::SetNextWindowSize(ImVec2(editorSize.x * editorCanvasSize.x, editorSize.y * editorCanvasSize.y));
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("BeeT Editor Window", &beetEditorWindowOpen, flags);

	ne::Begin("BeeT Node Editor");
	Menus();
	bt->Draw();	
	Links();

	// Node Editor Suspended -----------------------------
	ne::Suspend();
	ShowPopUps();
	
	if(widgetItemList->IsVisible())
		widgetItemList->Draw();
	ne::Resume();
	// ---------------------------------------------------

	ne::End(); // BeeT Node Editor
	ImGui::End();

	ImGui::PopStyleVar(); // WindowPadding
}

void BeeTEditor::Inspector()
{
	ImGui::SetNextWindowPos(ImVec2(screenWidth * (blackboardSize.x + editorCanvasSize.x), ImGui::GetCursorPosY() - ImGui::GetCursorPosX()));
	ImGui::SetNextWindowSize(ImVec2(editorSize.x * inspectorSize.x, editorSize.y * inspectorSize.y));
	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
	
	if (selectedNodeId != -1)
	{
		BTNode* nodeSel = bt->FindNode(selectedNodeId);
		if (nodeSel)
		{
			ImGui::Text("Type: %s", nodeSel->type->name.data());
			
			ImGui::Text("Name: "); ImGui::SameLine();
			char nodeNameTmp[_MAX_PATH];
			strcpy_s(nodeNameTmp, _MAX_PATH, nodeSel->name.data());
			ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_AutoSelectAll;
			if (nodeAddedFlag)
			{
				ImGui::SetKeyboardFocusHere();
				nodeAddedFlag = false;
			}
			if (ImGui::InputText("###", nodeNameTmp, _MAX_PATH, inputFlags))
			{
				nodeSel->name = nodeNameTmp;
			}
			
			ImGui::Spacing();
			ImGui::Text("Debug: ");
			ImGui::Spacing();
			ImGui::Separator();
			BTNode* parent = nodeSel->GetParent();
			if (parent)
				ImGui::Text("Parent: %i", parent->GetId());
			else
				ImGui::Text("Parent: -1");
			vector<BTNode*> childs = nodeSel->GetChilds();
			ImGui::Text("Childs: %i", childs.size());
			ImGui::Spacing();
			for (auto nodeChild : childs)
			{
				ImGui::Text("    * Node %i", nodeChild->GetId());
			}
			ImGui::Text("Subtree id: %i", nodeSel->GetSubtreeId());
			ImGui::Separator();

			ImGui::Text("Input: 0");
			ImGui::Text("Outputs: 0");
		}
	}

	ImGui::End();
}

void BeeTEditor::ShowPopUps()
{
	// Node Options
	if (ImGui::BeginPopup("Node options"))
	{
		ne::ClearSelection();
		ne::SelectNode(selectedNodeId);
		if (ImGui::MenuItem("Remove"))
		{
			bt->RemoveNode(selectedNodeId);
			selectedNodeId = -1;
			ne::ClearSelection();
		}
		ImGui::EndPopup();
	}

	// Link Options
	if (ImGui::BeginPopup("Link options"))
	{
		ne::ClearSelection();
		ne::SelectLink(selectedLinkId);
		if(ImGui::MenuItem("Remove"))
		{
			bt->RemoveLink(selectedLinkId);
			selectedLinkId = -1;
			ne::ClearSelection();
		}
		ImGui::EndPopup();
	}
}

void BeeTEditor::Menus()
{
	if (ne::ShowBackgroundContextMenu())
	{
		widgetItemList->SetVisible(true, g_app->beetGui->btNodeTypes->GetListObjectPtr());	// Activate ItemList
		widgetItemList->SetSelFunctionCallback(BeeTEditor::CallBackAddNode, this);			// Link Select Function
		ImVec2 mPos = ne::CanvasToScreen(ImGui::GetMousePos());
		widgetItemList->SetWidgetPosition(mPos.x, mPos.y);
	}
	if (ne::ShowNodeContextMenu(&selectedNodeId))
	{
		if(bt->IsRoot(selectedNodeId) == false)
			ImGui::OpenPopup("Node options");
		else
		{
			ne::ClearSelection();
			selectedNodeId = -1;
		}
	}
	if (ne::ShowLinkContextMenu(&selectedLinkId))
	{
		ImGui::OpenPopup("Link options");
	}
}

void BeeTEditor::Links()
{

	if (ne::BeginCreate(ImColor(255, 255, 255), 2.0f))
	{
		int startPinId = 0, endPinId = 0;
		if (ne::QueryNewLink(&startPinId, &endPinId))
		{
			BTPin* startPin = bt->FindPin(startPinId);
			BTPin* endPin = bt->FindPin(endPinId);

			if (startPin->kind == ne::PinKind::Target)
			{
				std::swap(startPin, endPin);
				std::swap(startPinId, endPinId);
			}

			if (startPin && endPin)
			{
				if (endPin->IsLinkAvailable() == false || startPin->IsLinkAvailable() == false)
				{
					// Maximum number of link connections made in input or output
					ne::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				else if (endPin == startPin || endPin->kind == startPin->kind || startPin->node == endPin->node)
				{
					// Same Pin || Same kind || Same node
					ne::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				else if (startPin->node->GetSubtreeId() == endPin->node->GetSubtreeId())
				{
					// Same tree it would create a loop
					ne::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				else if (ne::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
				{
					bt->AddLink(startPin, endPin);
				}
			}
		}
	}
	
	ne::EndCreate();
}

void BeeTEditor::UpdateSelection()
{
	// Get the node selected at this frame. ALWAYS remove root node from selection
	if (ne::HasSelectionChanged())
	{
		int selCount = ne::GetSelectedObjectCount();
		if (selCount == 1)
		{
			ne::GetSelectedNodes(&selectedNodeId, 1);
			if (bt->IsRoot(selectedNodeId))
			{
				selectedNodeId = -1;
				ne::ClearSelection();
			}
		}
		else
		{
			selectedNodeId = -1;
			vector<int> selNodes;
			selNodes.resize(selCount);

			ne::GetSelectedNodes(selNodes.data(), selCount);
			for (int i = 0; i < selCount; i++)
			{
				if (bt->IsRoot(selNodes[i]))
				{
					ne::DeselectNode(selNodes[i]);
					break;
				}
			}
		}
	}

}

void BeeTEditor::InitBBListCategories()
{
	bbVarListObj = new ListObject();

	bbVarListObj->AddItemInCategory("Basic", "Boolean");
	bbVarTypeConversor.insert(pair<string, BBVarType>("Boolean", BV_BOOL));
	bbVarListObj->AddItemInCategory("Basic", "Integer");
	bbVarTypeConversor.insert(pair<string, BBVarType>("Integer", BV_INT));
	bbVarListObj->AddItemInCategory("Basic", "Float");
	bbVarTypeConversor.insert(pair<string, BBVarType>("Float", BV_FLOAT));
	bbVarListObj->AddItemInCategory("Basic", "String");
	bbVarTypeConversor.insert(pair<string, BBVarType>("String", BV_STRING));

	bbVarListObj->SortAll();
}
