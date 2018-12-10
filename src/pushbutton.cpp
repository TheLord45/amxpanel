/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Andreas Theofilu and his suppliers, if any.
 * The intellectual and technical concepts contained
 * herein are proprietary to Andreas Theofilu and its suppliers and
 * may be covered by European and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Andreas Theofilu.
 */

#include "pushbutton.h"

using namespace amx;
using namespace strings;

PushButton::PushButton(const BUTTON_T& bt)
        : button(bt)
{
    onOff = false;
    state = 0;
}

String PushButton::getStyle()
{
    String style, bgcolor, fgcolor, fillcolor;
    style = "<style>\n";
    style += ".button {\n";
    style += "  background-color: ";

    if (button.type == GENERAL)
    {
        if (!onOff && button.sr.size() >= 1)
        {
            
        }
    }
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  margin: 4px 2px;
  cursor: pointer;
}

.button2 {background-color: #008CBA;} /* Blue */
.button3 {background-color: #f44336;} /* Red */ 
.button4 {background-color: #e7e7e7; color: black;} /* Gray */ 
.button5 {background-color: #555555;} /* Black */
</style>
}
