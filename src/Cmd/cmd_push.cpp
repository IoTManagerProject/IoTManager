#include "Cmd.h"

#include "PushingBox.h"

void cmd_push() {
    String title = sCmd.next();
    String body = sCmd.next();

    title.replace("#", " ");
    body.replace("#", " ");

    PushingBox::post(title, body);
}
