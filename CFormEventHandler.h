#pragma once
#define BUFSIZE 32

#include <list>
#include <Windows.h>

typedef struct customForm
{
	int _submitId;						// submit button의 ID
	char _method[BUFSIZE];				
	char _action[BUFSIZE];

	HWND _submitButton;					// submit button의 handle
	std::list<HWND> _hEdits;			// <form>에 포함되는 edit controls
	std::list<char*> _names;			// edit control마다 해당하는 name.
}customForm;

class CFormEventHandler
{
private:
	int m_formFlag;			// 현재 <form> 내부인지?
	int m_numControl;		// control의 총 개수 : ID를 저장해주기 위해서.
	char* m_bodyData;		// POST request에 필요한 body 데이터
	char* m_queryString;	// GET request에 필요한 queryString 
	char* m_action;			// submit button이 가지고 있는 action. -> submit button이 눌러졌을 때 이 변수에 action이 할당됨.

	std::list<customForm> m_forms;	// <form> 태그에 따른 구조체
	std::list<int> m_submitIds;		// 따로 submit button ID만 저장함. -> event 발생시 이 곳만 확인.

private:
	bool GetControlInfo(char* inputTag, char* typeBuf, char* nameBuf);		// ex)<input text= name= > 에서 text와 name을 파싱

public:
	CFormEventHandler();
	~CFormEventHandler();
	void Cleanup();

	void SetFormFlag(bool flag);													// <form> 태그가 왔을 시 true,
																					// </form> 태그일 경우 false.
																					// <form> </form> 사이에 오는 control들은 같은 form struct에 저장.
	bool MakeEventForm(char* formTag);												// <form>이 왔을 때 다음에 올 control들을 저장할 struct 생성
	HWND RegisterFormControls(char* inputTag, HWND hParent, HINSTANCE hInstance);	// form struct에 control들을 저장

	int EventProc(WPARAM wParam);													// submit button이 눌렸을 시 event 처리
	int GetNumForms();																// 총 control 개수 반환.
	bool MakePostBody(customForm iter);												// POST request 용 body 저장
	char* GetBodyData();
	int GetBodyLength();														
	char* GetAction();
	bool IsActionPushed(WPARAM wParam);												// submit button이 눌렸는지 확인
};