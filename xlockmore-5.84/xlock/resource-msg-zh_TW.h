/*
 * Resource messages for Traditional Chinese
 */

#ifndef __RESOURCE_MSG_ZH_TW__
#define __RESOURCE_MSG_ZH_TW__

#define DEF_NAME		"帳號："
#define DEF_PASS		"密碼："
#define DEF_VALID		"驗證登入中…"
#define DEF_INVALID		"無效的登入。"
#define DEF_INVALIDCAPSLOCK	"無效的登入，大寫鍵目前鎖定。"
#define DEF_INFO		"輸入密碼以解除鎖定；點選圖示以重新鎖定。"

#ifdef HAVE_KRB5
#define DEF_KRBINFO		"輸入 Kerberos 密碼以解除鎖定；點選圖示以重新鎖定。"
#endif /* HAVE_KRB5 */

#define DEF_COUNT_FAILED	" 次失敗。"
#define DEF_COUNTS_FAILED	" 次失敗。"

/* string that appears in logout button */
#define DEF_BTN_LABEL		"登出"

/* this string appears immediately below logout button */
#define DEF_BTN_HELP		"點選此處以登出"

/*
 * this string appears in place of the logout button
 * if user could not be logged out
 */
#define DEF_FAIL		"自動登出失敗"

#endif /* __RESOURCE_MSG_ZH_TW__ */
