#include "wallet_screen.h"
#include  "gfx/gfx_utils.h"


#define BUTTON_RADIUS           (BUTTON_HEIGHT / 2)
#define DOT_RADIUS              (3)
#define NUM_BUTTONS             (4)

static const WalletPaintColor ICON_COLOR = PW_TEAL;

extern const uint8_t QR_BUTTON[];
extern const uint8_t LOOP_BUTTON[];
extern const uint8_t PLUS_BUTTON[];
extern const uint8_t MINUS_BUTTON[];
extern const uint8_t ARROW_UP_BUTTON[];
extern const uint8_t ARROW_DOWN_BUTTON[];
extern const uint8_t ARROW_LEFT_BUTTON[];
extern const uint8_t ARROW_RIGHT_BUTTON[];
extern const uint8_t OK_BUTTON[];
extern const uint8_t MNEMONIC_BUTTON[];


void render_button_bar(const KeyButtonType types[]) {
    const WalletDisplayInfo* displayInfo = get_display_info();
    const uint8_t vPadding = 2;
    const uint16_t horizontalSpacing = (displayInfo->displayWidth / NUM_BUTTONS);
    
    uint16_t xPos = ((horizontalSpacing - BUTTON_HEIGHT) / 2);
    uint16_t yPos = displayInfo->displayHeight - BUTTON_HEIGHT - vPadding;
    
    for(int i = 0; i < NUM_BUTTONS; ++i, xPos += horizontalSpacing) {
        const uint8_t* bmpPtr = 0; 

        switch(types[i]) {
            case UP_KEY:
                bmpPtr = ARROW_UP_BUTTON;
                // wallet_gfx_draw_bitmap(ARROW_UP_BUTTON, xPos, yPos, BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case DOWN_KEY:
                bmpPtr = ARROW_DOWN_BUTTON;
                // wallet_gfx_draw_bitmap(ARROW_DOWN_BUTTON, xPos, yPos, BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case LEFT_KEY:
                bmpPtr = ARROW_LEFT_BUTTON;
                // wallet_gfx_draw_bitmap(ARROW_LEFT_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case RIGHT_KEY:
                bmpPtr = ARROW_RIGHT_BUTTON;
                // wallet_gfx_draw_bitmap(ARROW_RIGHT_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case MNEMONIC_KEY:
                bmpPtr = MNEMONIC_BUTTON;
                // wallet_gfx_draw_bitmap(MNEMONIC_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case OK_KEY:
                bmpPtr = OK_BUTTON;
                // wallet_gfx_draw_bitmap(OK_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case QR_CODE_KEY:
                bmpPtr = QR_BUTTON;
                // wallet_gfx_draw_bitmap(QR_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case LOOP_KEY:
                bmpPtr = LOOP_BUTTON;
                // wallet_gfx_draw_bitmap(LOOP_BUTTON, xPos, yPos, BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case PLUS_KEY:
                bmpPtr = PLUS_BUTTON;
                // wallet_gfx_draw_bitmap(PLUS_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case MINUS_KEY:
                bmpPtr = MINUS_BUTTON;
                // wallet_gfx_draw_bitmap(MINUS_BUTTON, (xPos - BUTTON_RADIUS), (yPos - BUTTON_RADIUS), BUTTON_HEIGHT, BUTTON_HEIGHT);
                break;
            case OTHER_KEY:
            case NO_KEY:
            default:
                break;
        }

        if(bmpPtr) {
            wallet_gfx_draw_bitmap(bmpPtr, xPos, yPos, BUTTON_HEIGHT, BUTTON_HEIGHT);
        }
    }
}
