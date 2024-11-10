#pragma once
#include "ui_header.h"
#include "manager.h"
#include "setting.h"
#include "table.h"
#include "client.h"
#include "history.h"
#include "sounds.h"
#include "autoSave.h"
#include <algorithm> // ���� std::sort
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <thread>
#include <dwmapi.h>


#pragma comment(lib, "Dwmapi.lib")

#define sideBar_w 162.0f
#define operator_h 100.0f

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

static vector<float> add_student_scores;
static Student add_student(NULL, "", add_student_scores);
static char add_name_buffer[16] = { 0 };
static char add_id_buffer[16] = { 0 };
static char add_course_name_buffer[16] = { 0 };


static vector<float> modify_student_scores;
static Student modify_student(NULL, "", modify_student_scores);
static char modify_name_buffer[16] = { 0 };

static string modify_course_name;
static char modify_course_name_buffer[16] = { 0 };

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
void detHotkey()
{
    //ctrl+z ��������

    if (ImGui::IsKeyPressed(ImGuiKey_Z) && ImGui::GetIO().KeyCtrl) {
        history.undo();
    }

    //ctrl+y ��������
    if (ImGui::IsKeyPressed(ImGuiKey_Y) && ImGui::GetIO().KeyCtrl) {
        history.redo();
    }

    //ctrl+s ����
    if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl) {
        manager.saveToFile();
        _beginthread(promptSaveSounds, 0, NULL);
    }
    //ctrl + shift + s ���Ϊ
    if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyShift) {
        manager.saveToFileAs();
        _beginthread(promptSaveSounds, 0, NULL);
    }
//ctrl+o ��
    if (ImGui::IsKeyPressed(ImGuiKey_O) && ImGui::GetIO().KeyCtrl) {
        //���
        if (history.getCanUndo())
        {
            client.show_save_prompt_window = 2;
            _beginthread(promptErrorSounds, 0, NULL);
        }
        else
        {
            client.show_load_error_window = manager.loadFromFile();
        }
    }

    //ctrl+n �½�
    if (ImGui::IsKeyPressed(ImGuiKey_N) && ImGui::GetIO().KeyCtrl) {
        //����Ƿ���δ������޸�
        if (history.getCanUndo())
        {
            client.show_save_prompt_window = 1;
            _beginthread(promptErrorSounds, 0, NULL);
        }
        else
        {
            manager.newFile();
            table.student_changed = true;
            //table.refreshTable();
        }
    }


    //ctrl+p ������
    if (ImGui::IsKeyPressed(ImGuiKey_P) && ImGui::GetIO().KeyCtrl) {
        client.show_settings_window = true;
        client.show_add_student_window = false;
        client.show_modify_student_window = false;
        client.show_add_course_window = false;
    }

}
// �Զ���������
bool SortById(const Student& a, const Student& b) { return a.id < b.id; }
//bool SortByName(const Student& a, const Student& b) { return a.name < b.name; }
bool SortByTotalScore(const Student& a, const Student& b) { return a.totalScore < b.totalScore; }
bool SortByCourseScore(const Student& a, const Student& b, int courseIndex)
{
    int scoreA = (courseIndex < a.scores.size()) ? a.scores[courseIndex] : -1;
    int scoreB = (courseIndex < b.scores.size()) ? b.scores[courseIndex] : -1;
    return scoreA < scoreB;
}

void sortTable()
{
    //�����������е�����ֻ����һ�Σ��Ż�����

    ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
    if (sortSpecs && !sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0 && !table.student_changed)
    {
        return;
    }
    const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0]; 
    bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending); // �����ǽ���
    // ��������

    switch (sortSpecs->Specs[0].ColumnIndex)
    {
    case 0: // ID ��
        if (ascending)
            sort(manager.students.begin(), manager.students.end(), SortById);
        else
            sort(manager.students.rbegin(), manager.students.rend(), SortById);
        break;
    //case 1: // Name ��
    //    if (ascending)
    //        sort(manager.students.begin(), manager.students.end(), SortByName);
    //    else
    //        sort(manager.students.rbegin(), manager.students.rend(), SortByName);
    //    break;
    default:
        if (sortSpecs->Specs[0].ColumnIndex == 2 + manager.courses.size()) // �ܷ���
        {
            if (ascending)
                sort(manager.students.begin(), manager.students.end(), SortByTotalScore);
            else
                sort(manager.students.rbegin(), manager.students.rend(), SortByTotalScore);
            break;
        }
        int courseIndex = spec.ColumnIndex - 2; // �γ���
        if (ascending)
            sort(manager.students.begin(), manager.students.end(), [courseIndex](const Student& a, const Student& b) {
            return SortByCourseScore(a, b, courseIndex);
                });
        else
            sort(manager.students.rbegin(), manager.students.rend(), [courseIndex](const Student& a, const Student& b) {
            return SortByCourseScore(a, b, courseIndex);
                });
        break;
    }
    table.refreshTable();
    sortSpecs->SpecsDirty = false;
    table.student_changed = false;
}

void renderTable()
{

    //static char searchBuffer[128] = "";
    //ImGui::InputText(u8"����", searchBuffer, IM_ARRAYSIZE(searchBuffer));
    ImGui::SetCursorPos({ 8.0f, 60.0f });
    ImGui::BeginChild(u8"���", ImVec2(client.current_window_size_pos.x - 185, client.current_window_size_pos.y - 69), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPos({ 0.0f, 0.0f });
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV /*| ImGuiTableFlags_NoBordersInBody */
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable(u8"ѧ���ɼ���", manager.courses.size() + 4, flags, ImVec2(client.current_window_size_pos.x - 185, client.current_window_size_pos.y - 69), 0.0f))
    {
        // ���Ʊ�ͷ
        ImGui::TableSetupColumn(u8"ѧ��", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 80.0f);
        // ���ݿγ�������̬���ƿγ���
        //for (int i = 0; i < manager.courses.size(); i++) {
        //        ImGui::TableSetupColumn(manager.courses[i].c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);

        //}
        for (vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
        {
            ImGui::TableSetupColumn(it->c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);
        }
        ImGui::TableSetupColumn(u8"�ܷ�", ImGuiTableColumnFlags_WidthStretch, 60.0f);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort);

        ImGui::TableHeadersRow();  // ������ͷ��

        sortTable();

        //table.refreshTable();

        // ���ѧ������
        // ���Ƹ���ƽ����
        if (setting.show_average_score)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text(u8"-");
            ImGui::TableNextColumn();
            ImGui::Text(u8"ƽ����");
            for(vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
            {
                ImGui::TableNextColumn();
                ImGui::Text(fmod(manager.averageScores[it - manager.courses.begin()], 1.0f) == 0.0f ? "%.0f" : "%.1f", manager.averageScores[it - manager.courses.begin()]);
            }
            //�ܷ�ƽ����
            ImGui::TableNextColumn();
            ImGui::Text(fmod(manager.totalAverageScore, 1.0f) == 0.0f ? "%.0f" : "%.1f", manager.totalAverageScore);

            ImGui::TableNextColumn();
            if(ImGui::Button(u8"����ƽ����"))
            {
                setting.show_average_score = false;
            }
        }
        bool filterChanged = false;
        for (auto& student : table.table_students)
        {
            ImGui::TableNextRow();

            // ��һ�У�ѧ��
            ImGui::TableNextColumn();
            ImGui::Text("%d", student.id);

            // �ڶ��У�����
            ImGui::TableNextColumn();
            ImGui::Text("%s", student.name.c_str());

            // �м��У����Ƴɼ�
            for (int i = 0; i < manager.courses.size(); i++)
            {
                if (i >= student.scores.size())
                {
                    ImGui::Text(u8"����");  // ����γ��������ڵ�ǰѧ���ķ���������ʾ����
                }
                //ȱ��
                if (student.scores[i] == -1)
                {
                    ImGui::TableNextColumn();
                    table.filter.markAbsent ? ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), u8"ȱ��") : ImGui::Text(u8"ȱ��");
                    continue;
                }
                //������
                if (table.filter.course_filters[i + 1].markFailed && student.scores[i] < table.filter.course_filters[i + 1].passLine)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), fmod(student.scores[i], 1.0f) == 0.0f ? "%.0f" : "%.1f", student.scores[i]);
                    continue;
                }
                //��ȱ���Ҽ���
                ImGui::TableNextColumn();
                ImGui::Text(fmod(student.scores[i], 1.0f) == 0.0f ? "%.0f" : "%.1f", student.scores[i]);
            }
            // �ܷ�
            if (table.filter.course_filters[0].markFailed && student.totalScore < table.filter.course_filters[0].passLine)            //������
            {
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), fmod(student.totalScore, 1.0f) == 0.0f ? "%.0f" : "%.1f", student.totalScore);
            }
            else
            {
                ImGui::TableNextColumn();
                ImGui::Text(fmod(student.totalScore, 1.0f) == 0.0f ? "%.0f" : "%.1f", student.totalScore);
            }
            //���һ�У�����
            ImGui::TableNextColumn();
            string modifyLable = u8"�޸�##" + to_string(student.id);
            if (ImGui::Button(modifyLable.c_str()))
            {
                modify_student = student;
                // ʹ�� copy ��������
                student.name.copy(modify_name_buffer, sizeof(modify_name_buffer) - 1);
                modify_name_buffer[sizeof(modify_name_buffer) - 1] = '\0';  // ȷ������ĩβΪ '\0'
                client.show_modify_student_window = true;
                client.setNextWindowTopMost();
            }
            ImGui::SameLine();
            string deletelable = u8"ɾ��##" + to_string(student.id);
            if (ImGui::Button(deletelable.c_str()))
            {
                manager.deleteStudent(student.id);
                filterChanged = true;
            }
        }

        ImGui::EndTable();  // �������
        if (filterChanged)
        {
            table.student_changed = true;
            //table.refreshTable();
        }
    }
    ImGui::EndChild(); //������
}

void renderSideBar()
{
    bool filterChanged = false;
    ImGui::SetCursorPos({ client.current_window_size_pos.x - sideBar_w - 8.0f, 60.0f });
    ImGui::BeginChild(u8"������", ImVec2(sideBar_w, operator_h), true);

    ImGui::BulletText(u8"������");
    ImGui::SameLine();
    ImGui::TextDisabled(setting.account.isLogin ? u8"�ѵ�¼" : u8"δ��¼");
    ImGui::Separator();
    if (ImGui::Button(u8"���ѧ��"))
    {
        add_student_scores.resize(manager.courses.size(), -1.0f);
        add_student = Student(NULL, "", add_student_scores);
        memset(add_name_buffer, 0, sizeof(add_name_buffer)); // �����������Ԫ������Ϊ '\0'
        memset(add_id_buffer, 0, sizeof(add_id_buffer)); // �����������Ԫ������Ϊ '\0'
        client.show_add_student_window = true;
        client.setNextWindowTopMost();
    }
    ImGui::SameLine();
    bool show_cs_warning = false;
    if (ImGui::Button(u8"���ѧ��"))
    {
        show_cs_warning = true;
        // client.setNextWindowTopMost();
        //manager.clearStudents();
        //filterChanged = true;
    }
    if (show_cs_warning)
    {
        ImGui::OpenPopup(u8"���ѧ������");
    }
    if (ImGui::BeginPopupModal(u8"���ѧ������", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"ȷ�����ѧ����Ϣ��\n�˲������޷�������");
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"ȷ��"))
        {
            manager.clearStudents();
            filterChanged = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"ȡ��").x - 24.0f);

        if (ImGui::Button(u8"ȡ��"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


    if (ImGui::Button(u8"��ӿ�Ŀ"))
    {
        memset(add_course_name_buffer, 0, sizeof(add_course_name_buffer)); // �����������Ԫ������Ϊ '\0'
        client.show_add_course_window = true;
        client.setNextWindowTopMost();
    }
    ImGui::SameLine();
    bool show_cc_warning = false;
    if (ImGui::Button(u8"��տ�Ŀ"))
    {
        show_cc_warning = true;
        // client.setNextWindowTopMost();
        //manager.clearStudents();
        //filterChanged = true;
    }
    if (show_cc_warning)
    {
        ImGui::OpenPopup(u8"��տγ̾���");
    }
    if (ImGui::BeginPopupModal(u8"��տγ̾���", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"ȷ��������п�Ŀ��\n�˲������޷�������");
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"ȷ��"))
        {
            manager.clearCourses();
            filterChanged = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"ȡ��").x - 24.0f);

        if (ImGui::Button(u8"ȡ��"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }

    ImGui::EndChild();//������

    ImGui::SetCursorPos({ client.current_window_size_pos.x - sideBar_w - 8.0f, 60.0f + operator_h + 8 });
    ImGui::BeginChild(u8"ɸѡ��", ImVec2(sideBar_w, client.current_window_size_pos.y - 69 - operator_h - 8), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::BulletText(u8"ɸѡ��");
    ImGui::Separator();
    if(ImGui::InputText(u8"����", table.filter.searchBuffer, IM_ARRAYSIZE(table.filter.searchBuffer)))
    {
        filterChanged = true;
    }
    if (ImGui::Button(u8"һ��������в�����"))
    {
        for (auto& course_filter : table.filter.course_filters)
        {
            course_filter.markFailed = true;
        }
        table.filter.markAbsent = true;
    }
    if (ImGui::TreeNodeEx(u8"ȱ��ɸѡ", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        ImGui::Unindent(ImGui::GetStyle().IndentSpacing);  // ȡ������
        if(ImGui::Checkbox(u8"ֻ��ʾȱ����", &table.filter.showAbsent))
        {
            filterChanged = true;
        }
        ImGui::Checkbox(u8"���ȱ��", &table.filter.markAbsent);
        ImGui::Indent(ImGui::GetStyle().IndentSpacing);  // �ָ�����
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx(u8"�ܷ�", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        ImGui::Unindent(ImGui::GetStyle().IndentSpacing);  // ȡ������

        if(ImGui::InputFloat(u8"����", &table.filter.course_filters[0].passLine, 0.0f, NULL, "%.1f"))
        {
            filterChanged = true;
        }
        if (ImGui::RadioButton(u8"��ɸѡ", table.filter.course_filters[0].showFailed == 0))
        {
            table.filter.course_filters[0].showFailed = 0;
            filterChanged = true;
        }
        if (ImGui::RadioButton(u8"ֻ��ʾ������", table.filter.course_filters[0].showFailed == 1))
        {
            table.filter.course_filters[0].showFailed = 1;
            filterChanged = true;
        }
        if (ImGui::RadioButton(u8"ֻ��ʾ����", table.filter.course_filters[0].showFailed == 2))
        {
            table.filter.course_filters[0].showFailed = 2;
            filterChanged = true;
        }

        ImGui::Checkbox(u8"��ǲ�����", &table.filter.course_filters[0].markFailed);

        ImGui::Indent(ImGui::GetStyle().IndentSpacing);  // �ָ�����
        ImGui::TreePop();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    float originalRounding = style.ChildRounding;
    style.ChildRounding = 0.0f;
    ImGui::SetCursorPosX(0.0f);
    ImGui::BeginChild(u8"�ɼ�ɸѡ", ImVec2(sideBar_w, client.current_window_size_pos.y - 69 - operator_h - 8 - ImGui::GetCursorPosY()), true);
    for (int i = 0; i < manager.courses.size(); i++)
    {
        string passLineLabel = u8"����##" + to_string(i);
        if (ImGui::TreeNodeEx(manager.courses[i].c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::Unindent(ImGui::GetStyle().IndentSpacing);  // ȡ������
            if (ImGui::InputFloat(passLineLabel.c_str(), &table.filter.course_filters[i + 1].passLine, 0.0f, NULL, "%.1f"))
            {
                filterChanged = true;
            }
            if (ImGui::RadioButton(u8"��ɸѡ", table.filter.course_filters[i + 1].showFailed == 0))
            {
                table.filter.course_filters[i + 1].showFailed = 0;
                filterChanged = true;
            }
            if (ImGui::RadioButton(u8"ֻ��ʾ������", table.filter.course_filters[i + 1].showFailed == 1))
            {
                table.filter.course_filters[i + 1].showFailed = 1;
                filterChanged = true;
            }
            if (ImGui::RadioButton(u8"ֻ��ʾ����", table.filter.course_filters[i + 1].showFailed == 2))
            {
                table.filter.course_filters[i + 1].showFailed = 2;
                filterChanged = true;
            }

            ImGui::Checkbox(u8"��ǲ�����", &table.filter.course_filters[i + 1].markFailed);

            if (ImGui::Button(u8"������"))
            {
                modify_course_name = manager.courses[i];
                // ʹ�� copy ��������
                modify_course_name.copy(modify_course_name_buffer, sizeof(modify_course_name_buffer) - 1);
                modify_course_name_buffer[sizeof(modify_course_name_buffer) - 1] = '\0';  // ȷ������ĩβΪ '\0'
                client.show_modify_course_window = true;
            }
            ImGui::SameLine();

            if (ImGui::Button(u8"ɾ����Ŀ"))
            {
                manager.deleteCourse(manager.courses[i]);
                filterChanged = true;
            }

            ImGui::Indent(ImGui::GetStyle().IndentSpacing);  // �ָ�����
            ImGui::TreePop();
        }
    }
    //���ð�ť����
    ImVec2 buttonPos = ImGui::GetCursorPos();
    //ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - (ImGui::GetWindowWidth() - 16.0f) / 2);
    if (ImGui::Button("##+", ImVec2(ImGui::GetWindowWidth() - 16.0f, 20.0f)))
    {
        client.show_add_course_window = true;
        client.setNextWindowTopMost();
    }
    //���üӺž���
    buttonPos.x = (ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"+").x) / 2;
    buttonPos.y -= 2.0f;
    ImGui::SetCursorPos(buttonPos);
    ImGui::Text(u8"+");

    ImGui::EndChild();// �ɼ�ɸѡ
    style.ChildRounding = originalRounding;

    ImGui::EndChild();//ɸѡ��
    if (filterChanged)
    {
        table.student_changed = true;
        //table.refreshTable();
    }
}

void renderSettingWindow(ImGuiIO& io)
{
    static bool isAutoSave = setting.autoSaveEnabled;

    static bool setting_setpos = false;
    if(!setting_setpos)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 350));
        //��ʾ������������λ��
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 200, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 175));

        setting_setpos = true;
    }
    ImGui::OpenPopup(u8"����");
    if (ImGui::BeginPopupModal(u8"����",NULL,  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"����");
        ImGui::Separator();
        ImGui::Checkbox(u8"��ʾƽ����", &setting.show_average_score);

        ImGui::SliderInt(u8"��೷������", &setting.maxHistorySize, 8, 256);

        if (ImGui::Checkbox(u8"�Զ�����", &isAutoSave))
        {
            toggleAutoSave(isAutoSave);
        }
        if (isAutoSave)
        {
            if (ImGui::Combo(u8"������", &setting.autoSaveIntervalType, u8"1����""\0""5����""\0""15����""\0""30����""\0""1Сʱ""\0", 5))
            {
                switch (setting.autoSaveIntervalType)
                {
                case AUTO_SAVE_1_MIN:
                    setAutoSaveInterval(1);
                    break;
                case AUTO_SAVE_5_MIN:
                    setAutoSaveInterval(5);
                    break;
                case AUTO_SAVE_15_MIN:
                    setAutoSaveInterval(15);
                    break;
                case AUTO_SAVE_30_MIN:
                    setAutoSaveInterval(30);
                    break;
                case AUTO_SAVE_1_HOUR:
                    setAutoSaveInterval(60);
                    break;
                }
            }
        }
        ImGui::Checkbox(u8"��ֱͬ��", &setting.vsync);
        if (!setting.vsync)
        {
            ImGui::Checkbox(u8"��֡", &setting.framerate_limit);
            if (setting.framerate_limit)
            {
                ImGui::SliderInt(u8"֡��", &setting.framerate_limit_value, 10, 360);
            }
        }
        ImGui::Checkbox(u8"ʧ����֡", &setting.unfocused_framerate_limit);
        ImGui::SameLine();
        HelpMarker(u8"������ʧȥ����ʱ������֡����10fps���ڣ�����CPUռ�á�");
        //ȷ��

        ImGui::SetCursorPos({ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2, ImGui::GetWindowHeight() - 40.0f});
        if (ImGui::Button(u8"ȷ��"))
        {
            ImGui::CloseCurrentPopup();
            client.show_settings_window = false;
            setting.saveSetting();
        }
        //��֡����ʾ�����Ͻ�
        string framerate_label = u8"֡�ʣ�" + to_string((int)io.Framerate);
        ImGui::SetCursorPos({ImGui::GetWindowWidth() - ImGui::CalcTextSize(framerate_label.c_str()).x - 8.0f, 8.0f});
        ImGui::TextDisabled(framerate_label.c_str());
        ImGui::EndPopup();
    }
    //ImGui::End();//���ô���

}

void renderAboutWindow()
{
    //static bool about_setpos = false;
    //if (!about_setpos)
    //{
    //    ImGui::SetNextWindowSize(ImVec2(360, 300));
    //    //��ʾ������������λ��
    //    ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 180, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 150));

    //    about_setpos = true;
    //}
    ImGui::OpenPopup(u8"����");
    if (ImGui::BeginPopupModal(u8"����", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::BulletText(u8"ѧ���ɼ�����ϵͳ");
        ImGui::Separator();
        ImGui::Text(u8"������");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"������");
        ImGui::Separator();
        ImGui::Text(u8"�汾��");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"1.0.0");
        ImGui::Separator();
        ImGui::Text(u8"���ԣ�");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"C++");
        ImGui::Separator();
        ImGui::Text(u8"UI�⣺");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"ImGui");
        ImGui::Separator();
        static int stu_count = 64;
        ImGui::Text(u8"��ʾ��");
        ImGui::SameLine();
        //ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_TextDisabled);
        if (ImGui::Button(u8"���ѧ�����Ŀ"))
        {
            manager.insertRandom(stu_count);
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::CalcTextSize(u8"���ѧ�����Ŀ").x + 8.0f);
        ImGui::SliderInt(u8"##���ѧ������", &stu_count, 1, 256, "%d");
        ImGui::PopItemWidth();
        //ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::Text(u8"˵����");

        ImGui::TextDisabled(u8"����һ���򵥵�ѧ���ɼ�����ϵͳ��");
        ImGui::TextDisabled(u8"�����ͨ�����뵼�����ܣ����ɼ�����򵼳����ļ���");
        ImGui::TextDisabled(u8"Ҳ����ͨ����ӡ�ɾ�����޸�ѧ���Ϳγ̣�������ɼ���");
        ImGui::Separator();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2);
        if (ImGui::Button(u8"ȷ��"))
        {
            ImGui::CloseCurrentPopup();
            client.show_about_window = false;
        }
        ImGui::EndPopup();
    }
}

void renderSavePromptWindow(bool &yes_close)
{
    ImGui::OpenPopup(u8"�Ƿ񱣴����?");
    if (ImGui::BeginPopupModal(u8"�Ƿ񱣴����?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"����δ������ģ��Ƿ񱣴棿");
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"����"))
        {
            manager.saveToFile();
            _beginthread(promptSaveSounds, 0, NULL);
            ImGui::CloseCurrentPopup();

            switch (client.show_save_prompt_window)
            {
            case 1: //�½�
                manager.clearStudents();
                manager.clearCourses();
                break;
            case 2: //��
                manager.loadFromFile();
                //table.student_changed = true;
                break;
            case 3: //�˳�
                yes_close = false;
                break;
            }
            client.show_save_prompt_window = 0;
        }
        ImGui::SameLine();
        //����
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"������").x / 2);

        if (ImGui::Button(u8"������"))
        {
            ImGui::CloseCurrentPopup();
            switch (client.show_save_prompt_window)
            {
            case 1: //�½�
                manager.clearStudents();
                manager.clearCourses();
                break;
            case 2: //��
                manager.loadFromFile();
                //table.student_changed = true;
                break;
            case 3: //�˳�
                yes_close = false;
                break;
            }
            client.show_save_prompt_window = 0;

        }
        ImGui::SameLine();
        //����
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"ȡ��").x - 24.0f);
        if (ImGui::Button(u8"ȡ��"))
        {
            ImGui::CloseCurrentPopup();
            client.show_save_prompt_window = 0;
        }
        ImGui::EndPopup();
    }
}

void renderLoadErrorWindow()
{
    static bool reasonAdded = false;
    ImGui::OpenPopup(u8"�ļ�����ʧ��");
    if (ImGui::BeginPopupModal(u8"�ļ�����ʧ��", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        static string error_Info = u8"�ļ�����ʧ�ܣ�";
        if (!reasonAdded)
        {
            switch (client.show_load_error_window)
            {
            case 1:
                error_Info += u8"�ļ������ڣ�";
                break;
            case 2:
                error_Info += u8"�ļ���ʽ����";
                break;
            }
            reasonAdded = true;
        }
        ImGui::BulletText(error_Info.c_str());
        ImGui::Separator();
        //����
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2);
        if (ImGui::Button(u8"ȷ��"))
        {
            ImGui::CloseCurrentPopup();
            error_Info = u8"�ļ�����ʧ�ܣ�";
            reasonAdded = false;
            client.show_load_error_window = 0;
        }
        ImGui::EndPopup();
    }
}

void renderAddStudentWindow()
{
    //vector<float> add_student_scores;
    //add_student_scores.resize(manager.courses.size(), -1.0f);

    //Student add_student(NULL, "", add_student_scores);
    static bool show_popup = false;
    static bool add_student_yes_close = true;
    static bool add_student_setpos = false;
    add_student_yes_close = client.show_add_student_window;
    if (!add_student_setpos)
    {
        ImGui::SetNextWindowSize(ImVec2(960, 98));
        //��ʾ������������λ��
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 480, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        add_student_setpos = true;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(512, 98), ImVec2(FLT_MAX, 98));
    ImGui::Begin(u8"���ѧ��", &add_student_yes_close, ImGuiWindowFlags_NoSavedSettings);

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV /*| ImGuiTableFlags_NoBordersInBody */
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable(u8"���ѧ���ɼ���", manager.courses.size() + 3, flags, ImVec2(ImGui::GetWindowWidth() - 16.0f, TEXT_BASE_HEIGHT * 2 + 8.0f)))
    {
        // ���Ʊ�ͷ
        ImGui::TableSetupColumn(u8"ѧ��", ImGuiTableColumnFlags_WidthStretch, 80.0f);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 80.0f);
        // ���ݿγ�������̬���ƿγ���

        for (vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
        {
            ImGui::TableSetupColumn(it->c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);
        }

        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 70.0f);

        ImGui::TableHeadersRow();  // ������ͷ��

        ImGui::TableNextRow();

        // ��һ�У�ѧ��
        ImGui::TableNextColumn();
        ImGui::InputText(u8"##ѧ��", add_id_buffer, IM_ARRAYSIZE(add_id_buffer), ImGuiInputTextFlags_CharsDecimal);

        add_student.id = atoi(add_id_buffer);

        // �ڶ��У�����
        ImGui::TableNextColumn();
        ImGui::InputText(u8"##����", add_name_buffer, IM_ARRAYSIZE(add_name_buffer));
        add_student.name = add_name_buffer;

        // �м��У����Ƴɼ�
        for (int i = 0; i < manager.courses.size(); i++)
        {
            ImGui::TableNextColumn();
            string score_label = u8"##" + to_string(i) + u8"�ɼ�";
            ImGui::InputFloat(score_label.c_str(), &add_student.scores[i], 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal);
        }
        static string error_Info;
        //���һ�У����
        ImGui::TableNextColumn();
        string deletelable = u8"���##";
        if (ImGui::Button(deletelable.c_str()))
        {
            add_student.calculateTotalScore();
            if (int errorType = manager.addStudent(add_student))
            {
                error_Info = u8"���ʧ�ܣ�";
                switch (errorType)
                {
                case 1:
                    error_Info += u8"ѧ���ظ���";
                    break;
                case 2:
                    error_Info += u8"�ɼ����Ϸ���";
                    break;
                case 3:
                    error_Info += u8"����Ϊ�գ�";
                    break;
                case 4:
                    error_Info += u8"ѧ�Ų��Ϸ���";
                    break;
                }
                //�������󴰿�
                show_popup = true;
                _beginthread(promptErrorSounds, 0, NULL);
            }
            else
            {
                table.student_changed = true;
                add_student_yes_close = false;
            }
        }
        if (show_popup) 
        {
            ImGui::OpenPopup(u8"����");
        }
        if(ImGui::BeginPopupModal(u8"����", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::BulletText(error_Info.c_str());
            ImGui::Separator();
            //ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2);
            ImGui::SetCursorPosX(16.0f);
            if (ImGui::Button(u8"�����޸�"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"�˳����").x - 24.0f);
            if (ImGui::Button(u8"�˳����"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
                add_student_yes_close = false;
            }

            ImGui::EndPopup();
        }
        ImGui::SameLine();
        HelpMarker(u8"���ѧ����Ϣ������ѧ�š����������Ƴɼ��������Ӱ�ť����ѧ�Ƴɼ�����-1��ʾȱ����");


        ImGui::EndTable();  // �������
    }

    ImGui::End();//���ô���
    if (!add_student_yes_close)
    {
        client.show_add_student_window = false;
    }
}

void renderModifyStudentWindow()
{
    //vector<float> add_student_scores;
    //add_student_scores.resize(manager.courses.size(), -1.0f);

    //Student add_student(NULL, "", add_student_scores);
    static bool show_popup = false;
    static bool modify_student_yes_close = true;
    static bool modify_student_setpos = false;
    modify_student_yes_close = client.show_modify_student_window;
    if (!modify_student_setpos)
    {
        ImGui::SetNextWindowSize(ImVec2(960, 98));
        //��ʾ������������λ��
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 480, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        modify_student_setpos = true;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(512, 98), ImVec2(FLT_MAX, 98));
    ImGui::Begin(u8"�޸�ѧ��", &modify_student_yes_close, ImGuiWindowFlags_NoSavedSettings);

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV /*| ImGuiTableFlags_NoBordersInBody */
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable(u8"�޸�ѧ���ɼ���", manager.courses.size() + 3, flags, ImVec2(ImGui::GetWindowWidth() - 16.0f, TEXT_BASE_HEIGHT * 2 + 8.0f)))
    {
        // ���Ʊ�ͷ
        ImGui::TableSetupColumn(u8"ѧ��", ImGuiTableColumnFlags_WidthStretch, 80.0f);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 80.0f);
        // ���ݿγ�������̬���ƿγ���

        for (vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
        {
            ImGui::TableSetupColumn(it->c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);
        }

        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 70.0f);

        ImGui::TableHeadersRow();  // ������ͷ��

        ImGui::TableNextRow();

        // ��һ�У�ѧ��
        ImGui::TableNextColumn();
        ImGui::Text("%d", modify_student.id);

        // �ڶ��У�����
        ImGui::TableNextColumn();
        ImGui::InputText(u8"##����", modify_name_buffer, IM_ARRAYSIZE(modify_name_buffer));
        modify_student.name = modify_name_buffer;

        // �м��У����Ƴɼ�
        for (int i = 0; i < manager.courses.size(); i++)
        {
            ImGui::TableNextColumn();
            string score_label = u8"##" + to_string(i) + u8"�ɼ�";
            ImGui::InputFloat(score_label.c_str(), &modify_student.scores[i], 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal);
        }
        static string error_Info;
        //���һ�У��޸�
        ImGui::TableNextColumn();
        string modifylable = u8"�޸�##";
        if (ImGui::Button(modifylable.c_str()))
        {
            modify_student.calculateTotalScore();
            if (int errorType = manager.modifyStudent(modify_student))
            {
                error_Info = u8"�޸�ʧ�ܣ�";
                switch (errorType)
                {
                //case 1:
                //    error_Info += u8"ѧ���ظ���";
                //    break;
                case 2:
                    error_Info += u8"�ɼ����Ϸ���";
                    break;
                case 3:
                    error_Info += u8"��������Ϊ�գ�";
                    break;
                }
                //�������󴰿�
                show_popup = true;
                _beginthread(promptErrorSounds, 0, NULL);
            }
            else
            {
                table.student_changed = true;
                modify_student_yes_close = false;
            }
        }
        if (show_popup)
        {
            ImGui::OpenPopup(u8"����");
        }
        if(ImGui::BeginPopupModal(u8"����", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::BulletText(error_Info.c_str());
            ImGui::Separator();
            //ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2);
            ImGui::SetCursorPosX(16.0f);
            if (ImGui::Button(u8"�����޸�"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"�˳��޸�").x - 24.0f);
            if (ImGui::Button(u8"�˳��޸�"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
                modify_student_yes_close = false;
            }

            ImGui::EndPopup();
        }
        ImGui::SameLine();
        HelpMarker(u8"�޸�ѧ����Ϣ���޸ġ����������Ƴɼ�������޸İ�ť����ѧ�Ƴɼ�����-1��ʾȱ����");


        ImGui::EndTable();  // �������
    }

    ImGui::End();//���ô���
    if (!modify_student_yes_close)
    {
        client.show_modify_student_window = false;
    }
}

void renderAddCourseWindow()
{
    static bool show_popup = false;
    static bool add_course_yes_close = true;
    static bool add_course_setpos = false;
    add_course_yes_close = client.show_add_course_window;
    static string error_Info;
    if (!add_course_setpos)
    {
        ImGui::SetNextWindowSize(ImVec2(240, 98));
        //��ʾ������������λ��
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 120, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        add_course_setpos = true;
    }

    ImGui::Begin(u8"��ӿ�Ŀ", &add_course_yes_close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
    //ImGui::BulletText(u8"��ӿ�Ŀ");
    //ImGui::Separator();
    ImGui::InputText(u8"��Ŀ����", add_course_name_buffer, IM_ARRAYSIZE(add_course_name_buffer));

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2);//����
    if (ImGui::Button(u8"ȷ��"))
    {
        if (int errorType = manager.addCourse(add_course_name_buffer))
        {
            error_Info = u8"���ʧ�ܣ�";
            switch (errorType)
            {
            case 1:
                error_Info += u8"��Ŀ������Ϊ�գ�";
                break;
            case 2:
                error_Info += u8"��Ŀ�����ظ���";
                break;
            }
            show_popup = true;
            _beginthread(promptErrorSounds, 0, NULL);
        }
        else
        {
            table.student_changed = true;
            add_course_yes_close = false;
        }
        ImGui::CloseCurrentPopup();
    }
    if (show_popup)
    {
        ImGui::OpenPopup(u8"��ӿ�Ŀ����");
    }
    if (ImGui::BeginPopupModal(u8"��ӿ�Ŀ����", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(error_Info.c_str());
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"�����޸�"))
        {
            ImGui::CloseCurrentPopup();
            show_popup = false;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"�˳��޸�").x - 24.0f);
        if (ImGui::Button(u8"�˳����"))
        {
            ImGui::CloseCurrentPopup();
            show_popup = false;
            add_course_yes_close = false;
        }

        ImGui::EndPopup();
    }
    if (!add_course_yes_close)
    {
        client.show_add_course_window = false;
    }
    ImGui::End();
}

void renderModifyCourseWindow()
{
    static bool show_popup = false;
    static bool modify_course_yes_close = true;
    static bool modify_course_setpos = false;
    modify_course_yes_close = client.show_modify_course_window;
    static string error_Info;
    if (!modify_course_setpos)
    {
        ImGui::SetNextWindowSize(ImVec2(240, 98));
        //��ʾ������������λ��
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 120, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        modify_course_setpos = true;
    }

    ImGui::Begin(u8"��������Ŀ", &modify_course_yes_close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
    //ImGui::BulletText(u8"��ӿ�Ŀ");
    //ImGui::Separator();
    ImGui::InputText(u8"��Ŀ����", modify_course_name_buffer, IM_ARRAYSIZE(modify_course_name_buffer));

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"ȷ��").x / 2);//����
    if (ImGui::Button(u8"ȷ��"))
    {
        if (int errorType = manager.modifyCourse(modify_course_name, modify_course_name_buffer))
        {
            error_Info = u8"��������Ŀʧ�ܣ�";
            switch (errorType)
            {
            case 1:
                error_Info += u8"��Ŀ������Ϊ�գ�";
                break;
            case 2:
                error_Info += u8"�޸ĺ�Ŀ�Ŀ������ԭ��Ŀ������ͬ��";
                break;
            case 3:
                error_Info += u8"��Ŀ�����ظ���";
                break;
            }
            show_popup = true;
            _beginthread(promptErrorSounds, 0, NULL);
        }
        else
        {
            table.student_changed = true;
            modify_course_yes_close = false;
        }
        ImGui::CloseCurrentPopup();
    }
    if (show_popup)
    {
        ImGui::OpenPopup(u8"��������Ŀ����");
    }
    if (ImGui::BeginPopupModal(u8"��������Ŀ����", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(error_Info.c_str());
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"�����޸�"))
        {
            ImGui::CloseCurrentPopup();
            show_popup = false;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"�˳��޸�").x - 24.0f);
        if (ImGui::Button(u8"�˳����"))
        {
            ImGui::CloseCurrentPopup();
            show_popup = false;
            modify_course_yes_close = false;
        }

        ImGui::EndPopup();
    }
    if (!modify_course_yes_close)
    {
        client.show_modify_course_window = false;
    }
    ImGui::End();
}

void renderMenuBar(bool &yes_close)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu(u8"�ļ�"))
        {
            if (ImGui::MenuItem(u8"�½�", "Ctrl+N")) 
            {
                //����Ƿ���δ������޸�
                if (history.getCanUndo())
                {
                    client.show_save_prompt_window = 1;
                    _beginthread(promptErrorSounds, 0, NULL);
                }
                else
                {
                    manager.newFile();
                    table.student_changed = true;
                }
            }

            if (ImGui::MenuItem(u8"��", "Ctrl+O")) 
            {
                if (history.getCanUndo())
                {
                    client.show_save_prompt_window = 2;
                }
                else
                {
                    client.show_load_error_window = manager.loadFromFile();
                }
                //table.student_changed = true;
            }
            if (ImGui::MenuItem(u8"����", "Ctrl+S")) 
            {
                manager.saveToFile();
                _beginthread(promptSaveSounds, 0, NULL);
            }
            if (ImGui::MenuItem(u8"���Ϊ", "Ctrl+Shift+S")) 
            {
                manager.saveToFileAs();
                _beginthread(promptSaveSounds, 0, NULL);
            }
            if (ImGui::MenuItem(u8"�˳�", "Alt+F4")) 
            {
                yes_close = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(u8"�༭"))
        {
            if (ImGui::MenuItem(u8"����", "CTRL+Z", false, history.getCanUndo()))
            {
                history.undo();
            }
            if (ImGui::MenuItem(u8"��ԭ", "CTRL+Y", false, history.getCanRedo()))
            {
                history.redo();
            }  // Disabled item
            //ImGui::Separator();
            //if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            //if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            //if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(u8"ѡ��"))
        {
            if (ImGui::MenuItem(u8"����")) 
            {
                client.show_settings_window = true;
                client.show_add_student_window = false;
                client.show_modify_student_window = false;
                client.show_add_course_window = false;
                //client.setNextWindowTopMost();
            }
            if (ImGui::MenuItem(u8"����")) 
            {
                client.show_about_window = true;
            }
            ImGui::SeparatorText(u8"�˻�");
            if(setting.account.isLogin)
            {
                if (ImGui::MenuItem(u8"ע��"))
                {
                    setting.account.isLogin = false;
                }
            }
            else
            {
                if (ImGui::MenuItem(u8"��¼"))
                {
                    setting.account.isLogin = true;
                }
            }

            ImGui::EndMenu();
        }
        string file_name;
        if (!setting.cur_file_name.empty())
        {
            file_name = setting.getCurFileName();
        }
        else
        {
            file_name = u8"δ����";
        }

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(file_name.c_str()).x - 16.0f);
        ImGui::TextDisabled(file_name.c_str());

        ImGui::EndMenuBar();
    }
}

void renderUI(ImGuiIO& io, bool& done)
{
    // Demonstrate the various window flags. Typically you would just use the default!
    static bool yes_close = !done;

    ImGuiWindowFlags window_flags = 0;
    //window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_MenuBar;
    //window_flags |= ImGuiWindowFlags_NoMove;
    //window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    //window_flags |= ImGuiWindowFlags_NoNav;
    //window_flags |= ImGuiWindowFlags_NoBackground;
    //window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoDocking;
    //window_flags |= ImGuiWindowFlags_UnsavedDocument;

    //launchAnime(setpos, 1.0f / io.Framerate);
    client.launchAnime(20.0f / client.getScreenRefreshRate());
// ������󻯶���
    if (client.isMaximizeAnime)
    {
        client.maximizeAnime(20.0f / client.getScreenRefreshRate());
    }
    else
    {
        client.setWindowContentSize(window_flags);
    }

    //string title = u8"ѧ���ɼ�����ϵͳ";
    //if (!manager.setting.cur_file_path.empty())
    //{
    //    title += u8" - " + manager.setting.getCurFileName();
    //}

    //ImGui::PushFont(font_big);
    // Main body of the Menu window starts here.
    if (!ImGui::Begin(u8"ѧ���ɼ�����ϵͳ", &yes_close, window_flags, true, true))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }
    //ImGui::PopFont();
    //ImVec2 cur_window_size = ImGui::GetWindowSize();
    client.current_window_size_pos.x = ImGui::GetWindowSize().x;
    client.current_window_size_pos.y = ImGui::GetWindowSize().y;
    client.current_window_size_pos.z = ImGui::GetWindowPos().x;
    client.current_window_size_pos.w = ImGui::GetWindowPos().y;

    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
    // ���Ʋ˵���
    renderMenuBar(yes_close);

    renderTable();

    renderSideBar();

    detHotkey();

    // �������ô���
    if (client.show_settings_window)
        renderSettingWindow(io);

    // �����޸Ŀγ̴���
    if (client.show_modify_course_window)
        renderModifyCourseWindow();


    // ���Ƽ��ش��󴰿�
    if (client.show_load_error_window && client.setpos)
        renderLoadErrorWindow();

    // ���ƹ��ڴ���
    if (client.show_about_window)
        renderAboutWindow();

    if (!yes_close && history.getCanUndo())
    {
        client.show_save_prompt_window = 3;
        client.show_add_student_window = false;
        client.show_modify_student_window = false;
        client.show_add_course_window = false;
        yes_close = true;
        _beginthread(promptErrorSounds, 0, NULL);
    }

    // ���Ʊ������Ѵ��� 
    if (client.show_save_prompt_window)
        renderSavePromptWindow(yes_close);

    done = !yes_close;

    ImGui::End(); // End of main body of the Menu window.



    // �������ѧ������
    if (client.show_add_student_window)
        renderAddStudentWindow();

    // �����޸�ѧ������
    if (client.show_modify_student_window)
        renderModifyStudentWindow();

    // ������ӿ�Ŀ����
    if (client.show_add_course_window)
        renderAddCourseWindow();
    //ImGui::ShowDemoWindow(&show_demo_window);

}