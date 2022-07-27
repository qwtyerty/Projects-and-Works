using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Valve.VR.Extras;
using UnityEngine.UI;
using Valve.VR.InteractionSystem;
using System;
using Valve.VR;


public class VRPointer : MonoBehaviour
{
    public SteamVR_LaserPointer pointer;
    public GameObject hand;
    public float returnSpeed;

    private void Awake()
    {
        //Attribute functions PointerIn, PointerOut and PointerClick to the pointer
        pointer.PointerClick += PointerClick;
        pointer.PointerOut += PointerOutside;
        pointer.PointerIn += PointerInside;
    }

    public void PointerClick(object sender, PointerEventArgs e)
    {
        Debug.Log(e.target.gameObject.name);
        //Only give velocity to Ions
        if (e.target.gameObject.tag == "Ion")
        {
            //Disable Gravity on the ion for better movement
            e.target.gameObject.GetComponent<Rigidbody>().useGravity = false;

            //Calculates a Vector3 for the direct of the clicked object to go
            var returnVector = hand.transform.position - e.target.gameObject.transform.position;

            //Add a velocity that has magnitude returnSpeed (Normalized Vectors have mag 1)
            e.target.gameObject.GetComponent<Rigidbody>().velocity = returnSpeed * returnVector.normalized;
        }
        else if (e.target.gameObject.tag == "Button")
        {
            e.target.gameObject.GetComponent<Button>().onClick.Invoke();
        }

    }

    public void PointerOutside(object sender, PointerEventArgs e)
    {
        if (e.target.gameObject.tag == "Ion")
            e.target.gameObject.GetComponent<IonBehavior>().isHovered = false;
    }

    public void PointerInside(object sender, PointerEventArgs e)
    {
        if (e.target.gameObject.tag == "Ion")
            e.target.gameObject.GetComponent<IonBehavior>().isHovered = true;
    }

}
