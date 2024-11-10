#pragma once
#include "ui_header.h"
#include "manager.h"
#include "setting.h"
#include "table.h"
#include "client.h"
#include "history.h"
#include "sounds.h"
#include "autoSave.h"
#include <algorithm> // 用于 std::sort
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
    //ctrl+z 触发撤回

    if (ImGui::IsKeyPressed(ImGuiKey_Z) && ImGui::GetIO().KeyCtrl) {
        history.undo();
    }

    //ctrl+y 触发重做
    if (ImGui::IsKeyPressed(ImGuiKey_Y) && ImGui::GetIO().KeyCtrl) {
        history.redo();
    }

    //ctrl+s 保存
    if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl) {
        manager.saveToFile();
        _beginthread(promptSaveSounds, 0, NULL);
    }
    //ctrl + shift + s 另存为
    if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyShift) {
        manager.saveToFileAs();
        _beginthread(promptSaveSounds, 0, NULL);
    }
//ctrl+o 打开
    if (ImGui::IsKeyPressed(ImGuiKey_O) && ImGui::GetIO().KeyCtrl) {
        //检测
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

    //ctrl+n 新建
    if (ImGui::IsKeyPressed(ImGuiKey_N) && ImGui::GetIO().KeyCtrl) {
        //检测是否有未保存的修改
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


    //ctrl+p 打开设置
    if (ImGui::IsKeyPressed(ImGuiKey_P) && ImGui::GetIO().KeyCtrl) {
        client.show_settings_window = true;
        client.show_add_student_window = false;
        client.show_modify_student_window = false;
        client.show_add_course_window = false;
    }

}
// 自定义排序函数
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
    //如果点击了这列的排序，只排序一次，优化性能

    ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
    if (sortSpecs && !sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0 && !table.student_changed)
    {
        return;
    }
    const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0]; 
    bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending); // 升序还是降序
    // 处理排序

    switch (sortSpecs->Specs[0].ColumnIndex)
    {
    case 0: // ID 列
        if (ascending)
            sort(manager.students.begin(), manager.students.end(), SortById);
        else
            sort(manager.students.rbegin(), manager.students.rend(), SortById);
        break;
    //case 1: // Name 列
    //    if (ascending)
    //        sort(manager.students.begin(), manager.students.end(), SortByName);
    //    else
    //        sort(manager.students.rbegin(), manager.students.rend(), SortByName);
    //    break;
    default:
        if (sortSpecs->Specs[0].ColumnIndex == 2 + manager.courses.size()) // 总分列
        {
            if (ascending)
                sort(manager.students.begin(), manager.students.end(), SortByTotalScore);
            else
                sort(manager.students.rbegin(), manager.students.rend(), SortByTotalScore);
            break;
        }
        int courseIndex = spec.ColumnIndex - 2; // 课程列
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
    //ImGui::InputText(u8"搜索", searchBuffer, IM_ARRAYSIZE(searchBuffer));
    ImGui::SetCursorPos({ 8.0f, 60.0f });
    ImGui::BeginChild(u8"表格", ImVec2(client.current_window_size_pos.x - 185, client.current_window_size_pos.y - 69), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPos({ 0.0f, 0.0f });
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV /*| ImGuiTableFlags_NoBordersInBody */
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable(u8"学生成绩表", manager.courses.size() + 4, flags, ImVec2(client.current_window_size_pos.x - 185, client.current_window_size_pos.y - 69), 0.0f))
    {
        // 绘制表头
        ImGui::TableSetupColumn(u8"学号", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(u8"姓名", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 80.0f);
        // 根据课程数量动态绘制课程列
        //for (int i = 0; i < manager.courses.size(); i++) {
        //        ImGui::TableSetupColumn(manager.courses[i].c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);

        //}
        for (vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
        {
            ImGui::TableSetupColumn(it->c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);
        }
        ImGui::TableSetupColumn(u8"总分", ImGuiTableColumnFlags_WidthStretch, 60.0f);
        ImGui::TableSetupColumn(u8"操作", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort);

        ImGui::TableHeadersRow();  // 创建表头行

        sortTable();

        //table.refreshTable();

        // 填充学生数据
        // 绘制各科平均分
        if (setting.show_average_score)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text(u8"-");
            ImGui::TableNextColumn();
            ImGui::Text(u8"平均分");
            for(vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
            {
                ImGui::TableNextColumn();
                ImGui::Text(fmod(manager.averageScores[it - manager.courses.begin()], 1.0f) == 0.0f ? "%.0f" : "%.1f", manager.averageScores[it - manager.courses.begin()]);
            }
            //总分平均分
            ImGui::TableNextColumn();
            ImGui::Text(fmod(manager.totalAverageScore, 1.0f) == 0.0f ? "%.0f" : "%.1f", manager.totalAverageScore);

            ImGui::TableNextColumn();
            if(ImGui::Button(u8"隐藏平均分"))
            {
                setting.show_average_score = false;
            }
        }
        bool filterChanged = false;
        for (auto& student : table.table_students)
        {
            ImGui::TableNextRow();

            // 第一列：学号
            ImGui::TableNextColumn();
            ImGui::Text("%d", student.id);

            // 第二列：姓名
            ImGui::TableNextColumn();
            ImGui::Text("%s", student.name.c_str());

            // 中间列：各科成绩
            for (int i = 0; i < manager.courses.size(); i++)
            {
                if (i >= student.scores.size())
                {
                    ImGui::Text(u8"错误");  // 如果课程数量多于当前学生的分数，则显示错误
                }
                //缺考
                if (student.scores[i] == -1)
                {
                    ImGui::TableNextColumn();
                    table.filter.markAbsent ? ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), u8"缺考") : ImGui::Text(u8"缺考");
                    continue;
                }
                //不及格
                if (table.filter.course_filters[i + 1].markFailed && student.scores[i] < table.filter.course_filters[i + 1].passLine)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), fmod(student.scores[i], 1.0f) == 0.0f ? "%.0f" : "%.1f", student.scores[i]);
                    continue;
                }
                //不缺考且及格
                ImGui::TableNextColumn();
                ImGui::Text(fmod(student.scores[i], 1.0f) == 0.0f ? "%.0f" : "%.1f", student.scores[i]);
            }
            // 总分
            if (table.filter.course_filters[0].markFailed && student.totalScore < table.filter.course_filters[0].passLine)            //不及格
            {
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), fmod(student.totalScore, 1.0f) == 0.0f ? "%.0f" : "%.1f", student.totalScore);
            }
            else
            {
                ImGui::TableNextColumn();
                ImGui::Text(fmod(student.totalScore, 1.0f) == 0.0f ? "%.0f" : "%.1f", student.totalScore);
            }
            //最后一列：操作
            ImGui::TableNextColumn();
            string modifyLable = u8"修改##" + to_string(student.id);
            if (ImGui::Button(modifyLable.c_str()))
            {
                modify_student = student;
                // 使用 copy 方法复制
                student.name.copy(modify_name_buffer, sizeof(modify_name_buffer) - 1);
                modify_name_buffer[sizeof(modify_name_buffer) - 1] = '\0';  // 确保数组末尾为 '\0'
                client.show_modify_student_window = true;
                client.setNextWindowTopMost();
            }
            ImGui::SameLine();
            string deletelable = u8"删除##" + to_string(student.id);
            if (ImGui::Button(deletelable.c_str()))
            {
                manager.deleteStudent(student.id);
                filterChanged = true;
            }
        }

        ImGui::EndTable();  // 结束表格
        if (filterChanged)
        {
            table.student_changed = true;
            //table.refreshTable();
        }
    }
    ImGui::EndChild(); //表格结束
}

void renderSideBar()
{
    bool filterChanged = false;
    ImGui::SetCursorPos({ client.current_window_size_pos.x - sideBar_w - 8.0f, 60.0f });
    ImGui::BeginChild(u8"操作栏", ImVec2(sideBar_w, operator_h), true);

    ImGui::BulletText(u8"操作栏");
    ImGui::SameLine();
    ImGui::TextDisabled(setting.account.isLogin ? u8"已登录" : u8"未登录");
    ImGui::Separator();
    if (ImGui::Button(u8"添加学生"))
    {
        add_student_scores.resize(manager.courses.size(), -1.0f);
        add_student = Student(NULL, "", add_student_scores);
        memset(add_name_buffer, 0, sizeof(add_name_buffer)); // 将数组的所有元素设置为 '\0'
        memset(add_id_buffer, 0, sizeof(add_id_buffer)); // 将数组的所有元素设置为 '\0'
        client.show_add_student_window = true;
        client.setNextWindowTopMost();
    }
    ImGui::SameLine();
    bool show_cs_warning = false;
    if (ImGui::Button(u8"清空学生"))
    {
        show_cs_warning = true;
        // client.setNextWindowTopMost();
        //manager.clearStudents();
        //filterChanged = true;
    }
    if (show_cs_warning)
    {
        ImGui::OpenPopup(u8"清空学生警告");
    }
    if (ImGui::BeginPopupModal(u8"清空学生警告", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"确定清空学生信息？\n此操作将无法撤销！");
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"确定"))
        {
            manager.clearStudents();
            filterChanged = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"取消").x - 24.0f);

        if (ImGui::Button(u8"取消"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


    if (ImGui::Button(u8"添加科目"))
    {
        memset(add_course_name_buffer, 0, sizeof(add_course_name_buffer)); // 将数组的所有元素设置为 '\0'
        client.show_add_course_window = true;
        client.setNextWindowTopMost();
    }
    ImGui::SameLine();
    bool show_cc_warning = false;
    if (ImGui::Button(u8"清空科目"))
    {
        show_cc_warning = true;
        // client.setNextWindowTopMost();
        //manager.clearStudents();
        //filterChanged = true;
    }
    if (show_cc_warning)
    {
        ImGui::OpenPopup(u8"清空课程警告");
    }
    if (ImGui::BeginPopupModal(u8"清空课程警告", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"确定清空所有科目？\n此操作将无法撤销！");
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"确定"))
        {
            manager.clearCourses();
            filterChanged = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"取消").x - 24.0f);

        if (ImGui::Button(u8"取消"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }

    ImGui::EndChild();//操作栏

    ImGui::SetCursorPos({ client.current_window_size_pos.x - sideBar_w - 8.0f, 60.0f + operator_h + 8 });
    ImGui::BeginChild(u8"筛选栏", ImVec2(sideBar_w, client.current_window_size_pos.y - 69 - operator_h - 8), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::BulletText(u8"筛选栏");
    ImGui::Separator();
    if(ImGui::InputText(u8"搜索", table.filter.searchBuffer, IM_ARRAYSIZE(table.filter.searchBuffer)))
    {
        filterChanged = true;
    }
    if (ImGui::Button(u8"一键标记所有不及格"))
    {
        for (auto& course_filter : table.filter.course_filters)
        {
            course_filter.markFailed = true;
        }
        table.filter.markAbsent = true;
    }
    if (ImGui::TreeNodeEx(u8"缺考筛选", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        ImGui::Unindent(ImGui::GetStyle().IndentSpacing);  // 取消缩进
        if(ImGui::Checkbox(u8"只显示缺考生", &table.filter.showAbsent))
        {
            filterChanged = true;
        }
        ImGui::Checkbox(u8"标记缺考", &table.filter.markAbsent);
        ImGui::Indent(ImGui::GetStyle().IndentSpacing);  // 恢复缩进
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx(u8"总分", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        ImGui::Unindent(ImGui::GetStyle().IndentSpacing);  // 取消缩进

        if(ImGui::InputFloat(u8"标线", &table.filter.course_filters[0].passLine, 0.0f, NULL, "%.1f"))
        {
            filterChanged = true;
        }
        if (ImGui::RadioButton(u8"不筛选", table.filter.course_filters[0].showFailed == 0))
        {
            table.filter.course_filters[0].showFailed = 0;
            filterChanged = true;
        }
        if (ImGui::RadioButton(u8"只显示不及格", table.filter.course_filters[0].showFailed == 1))
        {
            table.filter.course_filters[0].showFailed = 1;
            filterChanged = true;
        }
        if (ImGui::RadioButton(u8"只显示及格", table.filter.course_filters[0].showFailed == 2))
        {
            table.filter.course_filters[0].showFailed = 2;
            filterChanged = true;
        }

        ImGui::Checkbox(u8"标记不及格", &table.filter.course_filters[0].markFailed);

        ImGui::Indent(ImGui::GetStyle().IndentSpacing);  // 恢复缩进
        ImGui::TreePop();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    float originalRounding = style.ChildRounding;
    style.ChildRounding = 0.0f;
    ImGui::SetCursorPosX(0.0f);
    ImGui::BeginChild(u8"成绩筛选", ImVec2(sideBar_w, client.current_window_size_pos.y - 69 - operator_h - 8 - ImGui::GetCursorPosY()), true);
    for (int i = 0; i < manager.courses.size(); i++)
    {
        string passLineLabel = u8"标线##" + to_string(i);
        if (ImGui::TreeNodeEx(manager.courses[i].c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::Unindent(ImGui::GetStyle().IndentSpacing);  // 取消缩进
            if (ImGui::InputFloat(passLineLabel.c_str(), &table.filter.course_filters[i + 1].passLine, 0.0f, NULL, "%.1f"))
            {
                filterChanged = true;
            }
            if (ImGui::RadioButton(u8"不筛选", table.filter.course_filters[i + 1].showFailed == 0))
            {
                table.filter.course_filters[i + 1].showFailed = 0;
                filterChanged = true;
            }
            if (ImGui::RadioButton(u8"只显示不及格", table.filter.course_filters[i + 1].showFailed == 1))
            {
                table.filter.course_filters[i + 1].showFailed = 1;
                filterChanged = true;
            }
            if (ImGui::RadioButton(u8"只显示及格", table.filter.course_filters[i + 1].showFailed == 2))
            {
                table.filter.course_filters[i + 1].showFailed = 2;
                filterChanged = true;
            }

            ImGui::Checkbox(u8"标记不及格", &table.filter.course_filters[i + 1].markFailed);

            if (ImGui::Button(u8"重命名"))
            {
                modify_course_name = manager.courses[i];
                // 使用 copy 方法复制
                modify_course_name.copy(modify_course_name_buffer, sizeof(modify_course_name_buffer) - 1);
                modify_course_name_buffer[sizeof(modify_course_name_buffer) - 1] = '\0';  // 确保数组末尾为 '\0'
                client.show_modify_course_window = true;
            }
            ImGui::SameLine();

            if (ImGui::Button(u8"删除科目"))
            {
                manager.deleteCourse(manager.courses[i]);
                filterChanged = true;
            }

            ImGui::Indent(ImGui::GetStyle().IndentSpacing);  // 恢复缩进
            ImGui::TreePop();
        }
    }
    //设置按钮居中
    ImVec2 buttonPos = ImGui::GetCursorPos();
    //ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - (ImGui::GetWindowWidth() - 16.0f) / 2);
    if (ImGui::Button("##+", ImVec2(ImGui::GetWindowWidth() - 16.0f, 20.0f)))
    {
        client.show_add_course_window = true;
        client.setNextWindowTopMost();
    }
    //设置加号居中
    buttonPos.x = (ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"+").x) / 2;
    buttonPos.y -= 2.0f;
    ImGui::SetCursorPos(buttonPos);
    ImGui::Text(u8"+");

    ImGui::EndChild();// 成绩筛选
    style.ChildRounding = originalRounding;

    ImGui::EndChild();//筛选栏
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
        //显示在主窗口中心位置
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 200, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 175));

        setting_setpos = true;
    }
    ImGui::OpenPopup(u8"设置");
    if (ImGui::BeginPopupModal(u8"设置",NULL,  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"设置");
        ImGui::Separator();
        ImGui::Checkbox(u8"显示平均分", &setting.show_average_score);

        ImGui::SliderInt(u8"最多撤销次数", &setting.maxHistorySize, 8, 256);

        if (ImGui::Checkbox(u8"自动保存", &isAutoSave))
        {
            toggleAutoSave(isAutoSave);
        }
        if (isAutoSave)
        {
            if (ImGui::Combo(u8"保存间隔", &setting.autoSaveIntervalType, u8"1分钟""\0""5分钟""\0""15分钟""\0""30分钟""\0""1小时""\0", 5))
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
        ImGui::Checkbox(u8"垂直同步", &setting.vsync);
        if (!setting.vsync)
        {
            ImGui::Checkbox(u8"锁帧", &setting.framerate_limit);
            if (setting.framerate_limit)
            {
                ImGui::SliderInt(u8"帧率", &setting.framerate_limit_value, 10, 360);
            }
        }
        ImGui::Checkbox(u8"失焦限帧", &setting.unfocused_framerate_limit);
        ImGui::SameLine();
        HelpMarker(u8"当窗口失去焦点时，限制帧率在10fps以内，减少CPU占用。");
        //确定

        ImGui::SetCursorPos({ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2, ImGui::GetWindowHeight() - 40.0f});
        if (ImGui::Button(u8"确定"))
        {
            ImGui::CloseCurrentPopup();
            client.show_settings_window = false;
            setting.saveSetting();
        }
        //将帧率显示在右上角
        string framerate_label = u8"帧率：" + to_string((int)io.Framerate);
        ImGui::SetCursorPos({ImGui::GetWindowWidth() - ImGui::CalcTextSize(framerate_label.c_str()).x - 8.0f, 8.0f});
        ImGui::TextDisabled(framerate_label.c_str());
        ImGui::EndPopup();
    }
    //ImGui::End();//设置窗口

}

void renderAboutWindow()
{
    //static bool about_setpos = false;
    //if (!about_setpos)
    //{
    //    ImGui::SetNextWindowSize(ImVec2(360, 300));
    //    //显示在主窗口中心位置
    //    ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 180, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 150));

    //    about_setpos = true;
    //}
    ImGui::OpenPopup(u8"关于");
    if (ImGui::BeginPopupModal(u8"关于", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::BulletText(u8"学生成绩管理系统");
        ImGui::Separator();
        ImGui::Text(u8"制作：");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"吴彦组");
        ImGui::Separator();
        ImGui::Text(u8"版本：");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"1.0.0");
        ImGui::Separator();
        ImGui::Text(u8"语言：");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"C++");
        ImGui::Separator();
        ImGui::Text(u8"UI库：");
        ImGui::SameLine();
        ImGui::TextDisabled(u8"ImGui");
        ImGui::Separator();
        static int stu_count = 64;
        ImGui::Text(u8"演示：");
        ImGui::SameLine();
        //ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_TextDisabled);
        if (ImGui::Button(u8"随机学生与科目"))
        {
            manager.insertRandom(stu_count);
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::CalcTextSize(u8"随机学生与科目").x + 8.0f);
        ImGui::SliderInt(u8"##随机学生数量", &stu_count, 1, 256, "%d");
        ImGui::PopItemWidth();
        //ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::Text(u8"说明：");

        ImGui::TextDisabled(u8"这是一个简单的学生成绩管理系统。");
        ImGui::TextDisabled(u8"你可以通过导入导出功能，将成绩导入或导出到文件。");
        ImGui::TextDisabled(u8"也可以通过添加、删除、修改学生和课程，来管理成绩。");
        ImGui::Separator();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2);
        if (ImGui::Button(u8"确定"))
        {
            ImGui::CloseCurrentPopup();
            client.show_about_window = false;
        }
        ImGui::EndPopup();
    }
}

void renderSavePromptWindow(bool &yes_close)
{
    ImGui::OpenPopup(u8"是否保存更改?");
    if (ImGui::BeginPopupModal(u8"是否保存更改?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(u8"您尚未保存更改，是否保存？");
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"保存"))
        {
            manager.saveToFile();
            _beginthread(promptSaveSounds, 0, NULL);
            ImGui::CloseCurrentPopup();

            switch (client.show_save_prompt_window)
            {
            case 1: //新建
                manager.clearStudents();
                manager.clearCourses();
                break;
            case 2: //打开
                manager.loadFromFile();
                //table.student_changed = true;
                break;
            case 3: //退出
                yes_close = false;
                break;
            }
            client.show_save_prompt_window = 0;
        }
        ImGui::SameLine();
        //置中
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"不保存").x / 2);

        if (ImGui::Button(u8"不保存"))
        {
            ImGui::CloseCurrentPopup();
            switch (client.show_save_prompt_window)
            {
            case 1: //新建
                manager.clearStudents();
                manager.clearCourses();
                break;
            case 2: //打开
                manager.loadFromFile();
                //table.student_changed = true;
                break;
            case 3: //退出
                yes_close = false;
                break;
            }
            client.show_save_prompt_window = 0;

        }
        ImGui::SameLine();
        //靠右
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"取消").x - 24.0f);
        if (ImGui::Button(u8"取消"))
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
    ImGui::OpenPopup(u8"文件加载失败");
    if (ImGui::BeginPopupModal(u8"文件加载失败", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        static string error_Info = u8"文件加载失败，";
        if (!reasonAdded)
        {
            switch (client.show_load_error_window)
            {
            case 1:
                error_Info += u8"文件不存在！";
                break;
            case 2:
                error_Info += u8"文件格式错误！";
                break;
            }
            reasonAdded = true;
        }
        ImGui::BulletText(error_Info.c_str());
        ImGui::Separator();
        //置中
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2);
        if (ImGui::Button(u8"确定"))
        {
            ImGui::CloseCurrentPopup();
            error_Info = u8"文件加载失败，";
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
        //显示在主窗口中心位置
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 480, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        add_student_setpos = true;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(512, 98), ImVec2(FLT_MAX, 98));
    ImGui::Begin(u8"添加学生", &add_student_yes_close, ImGuiWindowFlags_NoSavedSettings);

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV /*| ImGuiTableFlags_NoBordersInBody */
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable(u8"添加学生成绩表", manager.courses.size() + 3, flags, ImVec2(ImGui::GetWindowWidth() - 16.0f, TEXT_BASE_HEIGHT * 2 + 8.0f)))
    {
        // 绘制表头
        ImGui::TableSetupColumn(u8"学号", ImGuiTableColumnFlags_WidthStretch, 80.0f);
        ImGui::TableSetupColumn(u8"姓名", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 80.0f);
        // 根据课程数量动态绘制课程列

        for (vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
        {
            ImGui::TableSetupColumn(it->c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);
        }

        ImGui::TableSetupColumn(u8"操作", ImGuiTableColumnFlags_WidthFixed, 70.0f);

        ImGui::TableHeadersRow();  // 创建表头行

        ImGui::TableNextRow();

        // 第一列：学号
        ImGui::TableNextColumn();
        ImGui::InputText(u8"##学号", add_id_buffer, IM_ARRAYSIZE(add_id_buffer), ImGuiInputTextFlags_CharsDecimal);

        add_student.id = atoi(add_id_buffer);

        // 第二列：姓名
        ImGui::TableNextColumn();
        ImGui::InputText(u8"##姓名", add_name_buffer, IM_ARRAYSIZE(add_name_buffer));
        add_student.name = add_name_buffer;

        // 中间列：各科成绩
        for (int i = 0; i < manager.courses.size(); i++)
        {
            ImGui::TableNextColumn();
            string score_label = u8"##" + to_string(i) + u8"成绩";
            ImGui::InputFloat(score_label.c_str(), &add_student.scores[i], 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal);
        }
        static string error_Info;
        //最后一列：添加
        ImGui::TableNextColumn();
        string deletelable = u8"添加##";
        if (ImGui::Button(deletelable.c_str()))
        {
            add_student.calculateTotalScore();
            if (int errorType = manager.addStudent(add_student))
            {
                error_Info = u8"添加失败，";
                switch (errorType)
                {
                case 1:
                    error_Info += u8"学号重复！";
                    break;
                case 2:
                    error_Info += u8"成绩不合法！";
                    break;
                case 3:
                    error_Info += u8"姓名为空！";
                    break;
                case 4:
                    error_Info += u8"学号不合法！";
                    break;
                }
                //弹出错误窗口
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
            ImGui::OpenPopup(u8"错误");
        }
        if(ImGui::BeginPopupModal(u8"错误", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::BulletText(error_Info.c_str());
            ImGui::Separator();
            //ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2);
            ImGui::SetCursorPosX(16.0f);
            if (ImGui::Button(u8"返回修改"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"退出添加").x - 24.0f);
            if (ImGui::Button(u8"退出添加"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
                add_student_yes_close = false;
            }

            ImGui::EndPopup();
        }
        ImGui::SameLine();
        HelpMarker(u8"添加学生信息：输入学号、姓名、各科成绩，点击添加按钮。（学科成绩输入-1表示缺考）");


        ImGui::EndTable();  // 结束表格
    }

    ImGui::End();//设置窗口
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
        //显示在主窗口中心位置
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 480, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        modify_student_setpos = true;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(512, 98), ImVec2(FLT_MAX, 98));
    ImGui::Begin(u8"修改学生", &modify_student_yes_close, ImGuiWindowFlags_NoSavedSettings);

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV /*| ImGuiTableFlags_NoBordersInBody */
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable(u8"修改学生成绩表", manager.courses.size() + 3, flags, ImVec2(ImGui::GetWindowWidth() - 16.0f, TEXT_BASE_HEIGHT * 2 + 8.0f)))
    {
        // 绘制表头
        ImGui::TableSetupColumn(u8"学号", ImGuiTableColumnFlags_WidthStretch, 80.0f);
        ImGui::TableSetupColumn(u8"姓名", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 80.0f);
        // 根据课程数量动态绘制课程列

        for (vector<string>::const_iterator it = manager.courses.begin(); it != manager.courses.end(); ++it)
        {
            ImGui::TableSetupColumn(it->c_str(), ImGuiTableColumnFlags_WidthStretch, 60.0f);
        }

        ImGui::TableSetupColumn(u8"操作", ImGuiTableColumnFlags_WidthFixed, 70.0f);

        ImGui::TableHeadersRow();  // 创建表头行

        ImGui::TableNextRow();

        // 第一列：学号
        ImGui::TableNextColumn();
        ImGui::Text("%d", modify_student.id);

        // 第二列：姓名
        ImGui::TableNextColumn();
        ImGui::InputText(u8"##姓名", modify_name_buffer, IM_ARRAYSIZE(modify_name_buffer));
        modify_student.name = modify_name_buffer;

        // 中间列：各科成绩
        for (int i = 0; i < manager.courses.size(); i++)
        {
            ImGui::TableNextColumn();
            string score_label = u8"##" + to_string(i) + u8"成绩";
            ImGui::InputFloat(score_label.c_str(), &modify_student.scores[i], 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal);
        }
        static string error_Info;
        //最后一列：修改
        ImGui::TableNextColumn();
        string modifylable = u8"修改##";
        if (ImGui::Button(modifylable.c_str()))
        {
            modify_student.calculateTotalScore();
            if (int errorType = manager.modifyStudent(modify_student))
            {
                error_Info = u8"修改失败，";
                switch (errorType)
                {
                //case 1:
                //    error_Info += u8"学号重复！";
                //    break;
                case 2:
                    error_Info += u8"成绩不合法！";
                    break;
                case 3:
                    error_Info += u8"姓名不能为空！";
                    break;
                }
                //弹出错误窗口
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
            ImGui::OpenPopup(u8"错误");
        }
        if(ImGui::BeginPopupModal(u8"错误", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::BulletText(error_Info.c_str());
            ImGui::Separator();
            //ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2);
            ImGui::SetCursorPosX(16.0f);
            if (ImGui::Button(u8"返回修改"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"退出修改").x - 24.0f);
            if (ImGui::Button(u8"退出修改"))
            {
                ImGui::CloseCurrentPopup();
                show_popup = false;
                modify_student_yes_close = false;
            }

            ImGui::EndPopup();
        }
        ImGui::SameLine();
        HelpMarker(u8"修改学生信息：修改、姓名、各科成绩，点击修改按钮。（学科成绩输入-1表示缺考）");


        ImGui::EndTable();  // 结束表格
    }

    ImGui::End();//设置窗口
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
        //显示在主窗口中心位置
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 120, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        add_course_setpos = true;
    }

    ImGui::Begin(u8"添加科目", &add_course_yes_close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
    //ImGui::BulletText(u8"添加科目");
    //ImGui::Separator();
    ImGui::InputText(u8"科目名称", add_course_name_buffer, IM_ARRAYSIZE(add_course_name_buffer));

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2);//居中
    if (ImGui::Button(u8"确定"))
    {
        if (int errorType = manager.addCourse(add_course_name_buffer))
        {
            error_Info = u8"添加失败，";
            switch (errorType)
            {
            case 1:
                error_Info += u8"科目名不能为空！";
                break;
            case 2:
                error_Info += u8"科目名称重复！";
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
        ImGui::OpenPopup(u8"添加科目错误");
    }
    if (ImGui::BeginPopupModal(u8"添加科目错误", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(error_Info.c_str());
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"返回修改"))
        {
            ImGui::CloseCurrentPopup();
            show_popup = false;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"退出修改").x - 24.0f);
        if (ImGui::Button(u8"退出添加"))
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
        //显示在主窗口中心位置
        ImGui::SetNextWindowPos(ImVec2(client.current_window_size_pos.z + client.current_window_size_pos.x / 2 - 120, client.current_window_size_pos.w + client.current_window_size_pos.y / 2 - 49));
        modify_course_setpos = true;
    }

    ImGui::Begin(u8"重命名科目", &modify_course_yes_close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
    //ImGui::BulletText(u8"添加科目");
    //ImGui::Separator();
    ImGui::InputText(u8"科目名称", modify_course_name_buffer, IM_ARRAYSIZE(modify_course_name_buffer));

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(u8"确定").x / 2);//居中
    if (ImGui::Button(u8"确定"))
    {
        if (int errorType = manager.modifyCourse(modify_course_name, modify_course_name_buffer))
        {
            error_Info = u8"重命名科目失败，";
            switch (errorType)
            {
            case 1:
                error_Info += u8"科目名不能为空！";
                break;
            case 2:
                error_Info += u8"修改后的科目名称与原科目名称相同！";
                break;
            case 3:
                error_Info += u8"科目名称重复！";
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
        ImGui::OpenPopup(u8"重命名科目错误");
    }
    if (ImGui::BeginPopupModal(u8"重命名科目错误", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::BulletText(error_Info.c_str());
        ImGui::Separator();
        ImGui::SetCursorPosX(16.0f);
        if (ImGui::Button(u8"返回修改"))
        {
            ImGui::CloseCurrentPopup();
            show_popup = false;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"退出修改").x - 24.0f);
        if (ImGui::Button(u8"退出添加"))
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
        if (ImGui::BeginMenu(u8"文件"))
        {
            if (ImGui::MenuItem(u8"新建", "Ctrl+N")) 
            {
                //检测是否有未保存的修改
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

            if (ImGui::MenuItem(u8"打开", "Ctrl+O")) 
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
            if (ImGui::MenuItem(u8"保存", "Ctrl+S")) 
            {
                manager.saveToFile();
                _beginthread(promptSaveSounds, 0, NULL);
            }
            if (ImGui::MenuItem(u8"另存为", "Ctrl+Shift+S")) 
            {
                manager.saveToFileAs();
                _beginthread(promptSaveSounds, 0, NULL);
            }
            if (ImGui::MenuItem(u8"退出", "Alt+F4")) 
            {
                yes_close = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(u8"编辑"))
        {
            if (ImGui::MenuItem(u8"撤销", "CTRL+Z", false, history.getCanUndo()))
            {
                history.undo();
            }
            if (ImGui::MenuItem(u8"还原", "CTRL+Y", false, history.getCanRedo()))
            {
                history.redo();
            }  // Disabled item
            //ImGui::Separator();
            //if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            //if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            //if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(u8"选项"))
        {
            if (ImGui::MenuItem(u8"设置")) 
            {
                client.show_settings_window = true;
                client.show_add_student_window = false;
                client.show_modify_student_window = false;
                client.show_add_course_window = false;
                //client.setNextWindowTopMost();
            }
            if (ImGui::MenuItem(u8"关于")) 
            {
                client.show_about_window = true;
            }
            ImGui::SeparatorText(u8"账户");
            if(setting.account.isLogin)
            {
                if (ImGui::MenuItem(u8"注销"))
                {
                    setting.account.isLogin = false;
                }
            }
            else
            {
                if (ImGui::MenuItem(u8"登录"))
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
            file_name = u8"未保存";
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
// 窗口最大化动画
    if (client.isMaximizeAnime)
    {
        client.maximizeAnime(20.0f / client.getScreenRefreshRate());
    }
    else
    {
        client.setWindowContentSize(window_flags);
    }

    //string title = u8"学生成绩管理系统";
    //if (!manager.setting.cur_file_path.empty())
    //{
    //    title += u8" - " + manager.setting.getCurFileName();
    //}

    //ImGui::PushFont(font_big);
    // Main body of the Menu window starts here.
    if (!ImGui::Begin(u8"学生成绩管理系统", &yes_close, window_flags, true, true))
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
    // 绘制菜单栏
    renderMenuBar(yes_close);

    renderTable();

    renderSideBar();

    detHotkey();

    // 绘制设置窗口
    if (client.show_settings_window)
        renderSettingWindow(io);

    // 绘制修改课程窗口
    if (client.show_modify_course_window)
        renderModifyCourseWindow();


    // 绘制加载错误窗口
    if (client.show_load_error_window && client.setpos)
        renderLoadErrorWindow();

    // 绘制关于窗口
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

    // 绘制保存提醒窗口 
    if (client.show_save_prompt_window)
        renderSavePromptWindow(yes_close);

    done = !yes_close;

    ImGui::End(); // End of main body of the Menu window.



    // 绘制添加学生窗口
    if (client.show_add_student_window)
        renderAddStudentWindow();

    // 绘制修改学生窗口
    if (client.show_modify_student_window)
        renderModifyStudentWindow();

    // 绘制添加科目窗口
    if (client.show_add_course_window)
        renderAddCourseWindow();
    //ImGui::ShowDemoWindow(&show_demo_window);

}