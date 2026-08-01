// roundtrip_test.cpp -- standalone, headless verification tool for the
// player-input event round-trip (BasicEntityEvent -> ZCom_BitStream ->
// BasicEntityEvent) that HovercraftController.cpp/Entity.cpp rely on every
// physics step to move the player-controlled and AI-controlled hovercrafts.
//
// Written in response to a bug report: arrow-up moves backward, arrow-down
// moves forward, and left/right steering does nothing at all -- the
// classic signature of a bit-misalignment bug in a hand-rolled bitstream
// codec (a reader offset by a bit or two relative to the writer decodes
// neighbouring flags as each other, or reads a stale/zeroed bit for a flag
// that was actually set). This exact class of defect was already found and
// fixed once today in this same file's ZCom_BitStream::getString() (a stub
// that silently consumed 0 bits and desynced every subsequent read), so it
// was flagged as the prime suspect for this bug too.
//
// This tool drives the EXACT code path the game uses -- BasicEntityEvent's
// NetworkEvent<>::serialize()/deserialize() (event-class tag + type tag +
// write()/read()) through a real ZCom_BitStream, then back out through
// ControllerEventParser::parse() (checkEventClass/readType/dispatch), the
// same parser Entity::parseEvents() calls for every incoming network event
// -- for all 32 possible forward/backward/left/right/reset combinations.
//
// Not part of the shipped game; not linked into HovercraftUniverse.exe.
#include <BasicEntityEvent.h>
#include <ControllerEventParser.h>
#include <zoidcom/zoidcom.h>

#include <iostream>

using namespace HovUni;

int main() {
    int failures = 0;

    for (int mask = 0; mask < 32; ++mask) {
        bool fwd = (mask & 1) != 0;
        bool bwd = (mask & 2) != 0;
        bool left = (mask & 4) != 0;
        bool right = (mask & 8) != 0;
        bool reset = (mask & 16) != 0;

        BasicEntityEvent original(fwd, bwd, left, right, reset);

        ZCom_BitStream stream;
        original.serialize(&stream);

        // Exercise the exact receive-side path: ControllerEventParser::parse()
        // peeks the event class (checkEventClass), peeks+reads the type
        // (readType), then dispatches to BasicEntityEvent::parse(), which
        // calls deserialize() -- exactly what Entity::parseEvents() does for
        // every incoming eZCom_EventUser event.
        ControllerEventParser parser;
        ControllerEvent* parsedEvent = parser.parse(&stream);
        BasicEntityEvent* parsed = dynamic_cast<BasicEntityEvent*>(parsedEvent);

        bool ok = parsed != nullptr
            && parsed->moveForward() == fwd
            && parsed->moveBackward() == bwd
            && parsed->moveLeft() == left
            && parsed->moveRight() == right
            && parsed->reset() == reset;

        if (!ok) {
            failures++;
            std::cout << "[roundtrip_test] FAIL mask=" << mask
                      << " wrote(fwd=" << fwd << ",bwd=" << bwd << ",left=" << left
                      << ",right=" << right << ",reset=" << reset << ")";
            if (parsed) {
                std::cout << " read(fwd=" << parsed->moveForward() << ",bwd=" << parsed->moveBackward()
                          << ",left=" << parsed->moveLeft() << ",right=" << parsed->moveRight()
                          << ",reset=" << parsed->reset() << ")";
            } else {
                std::cout << " read=NULL (parse failed / wrong dynamic type)";
            }
            std::cout << std::endl;
        }

        delete parsedEvent;
    }

    std::cout << "[roundtrip_test] " << (32 - failures) << "/32 combinations round-tripped correctly" << std::endl;
    std::cout << "[roundtrip_test] " << (failures == 0 ? "ALL OK" : "FAILURES PRESENT") << std::endl;
    return failures == 0 ? 0 : 1;
}
